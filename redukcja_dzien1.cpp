#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono> // Potrzebne do pomiaru czasu

using namespace std;

// Zwiększamy rozmiar, aby procesor miał co robić przez zauważalny czas
const int ROZMIAR = 50000000; 

vector<int> dane(ROZMIAR);
mutex mtx;

// --- METODA 1: LOKALNA AKUMULACJA (SZYBKA) ---
// Wątek liczy wszystko u siebie, a na koniec tylko raz "idzie" do pamięci głównej
void przetwarzaj_mutex(int start, int end, const vector<int>& dane, int& globalny_licznik) {
    int local_count = 0; // Zmienna lokalna - procesor trzyma ją w super-szybkim rejestrze
    
    for (int i = start; i < end; ++i) {
        if (dane[i] > 5000) { // Warunek spełniony w ok. 50% przypadków
            local_count++;
        }
    }

    // SEKRETY WYDAJNOŚCI: Blokujemy mutex TYLKO RAZ na cały wątek.
    // To minimalizuje rywalizację (contention) między wątkami.
    lock_guard<mutex> lock(mtx);
    globalny_licznik += local_count;
}

// --- METODA 2: CIĄGŁY ATOMIC (WOLNA) ---
// Wątek przy każdym trafieniu musi komunikować się z szyną pamięci RAM
void przetwarzaj_atomic(int start, int end, const vector<int>& dane, atomic<int>& globalny_atomic) {
    for (int i = start; i < end; ++i) {
        if (dane[i] > 5000) {
            // fetch_add tutaj wykonuje się miliony razy.
            // Każdy wątek zmusza inne wątki do odświeżenia ich pamięci podręcznej (Cache).
            globalny_atomic.fetch_add(1);
        }
    }
}

int main() {
    // Inicjalizacja danych
    cout << "Generowanie danych..." << endl;
    for (int i = 0; i < ROZMIAR; ++i) {
        dane[i] = rand() % 10001;
    }

    int liczba_watkow = thread::hardware_concurrency(); // Pobiera liczbę rdzeni Twojego CPU
    if (liczba_watkow == 0) liczba_watkow = 4;
    
    vector<thread> watki;

    // --- TEST 1: MUTEX + LOKALNY LICZNIK ---
    int wynik_mutex = 0;
    cout << "Start: Metoda Mutex (Lokalny licznik)..." << endl;
    auto s1 = chrono::high_resolution_clock::now();

    for (int i = 0; i < liczba_watkow; i++) {
        watki.emplace_back(przetwarzaj_mutex, (ROZMIAR / liczba_watkow) * i, (ROZMIAR / liczba_watkow) * (i + 1), ref(dane), ref(wynik_mutex));
    }
    for (auto& t : watki) t.join(); // Czekamy na koniec pierwszej grupy

    auto e1 = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> czas_mutex = e1 - s1;

    // --- TEST 2: CIĄGŁY ATOMIC ---
    watki.clear();
    atomic<int> wynik_atomic(0);
    cout << "Start: Metoda Atomic (Czegste aktualizacje)..." << endl;
    auto s2 = chrono::high_resolution_clock::now();

    for (int i = 0; i < liczba_watkow; i++) {
        watki.emplace_back(przetwarzaj_atomic, (ROZMIAR / liczba_watkow) * i, (ROZMIAR / liczba_watkow) * (i + 1), ref(dane), ref(wynik_atomic));
    }
    for (auto& t : watki) t.join(); // Czekamy na koniec drugiej grupy

    auto e2 = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> czas_atomic = e2 - s2;

    // --- PODSUMOWANIE ---
    cout << "\nWyniki:" << endl;
    cout << "Metoda Mutex:  " << wynik_mutex << " trafien, czas: " << czas_mutex.count() << " ms" << endl;
    cout << "Metoda Atomic: " << wynik_atomic.load() << " trafien, czas: " << czas_atomic.count() << " ms" << endl;
    
    cout << "\nMetoda z lokalnym licznikiem jest " << czas_atomic.count() / czas_mutex.count() << "x szybsza!" << endl;

    return 0;
}