#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <queue>
#include <condition_variable>
#include <future>
#include <string>

using namespace std;

// --- ZASOBY WSPÓŁDZIELONE ---
queue<string> logi;
mutex mtx_logi;
condition_variable cv_logi;
bool czy_koniec = false;

// --- 1. MONITOR (Wątek odłączony - "Daemon") ---
void miejski_monitoring() {
    while (true) {
        // Monitor działa w tle co 1.5 sekundy
        this_thread::sleep_for(chrono::milliseconds(1500));
        // Używamy locka nawet do prostej wypisywki, żeby napisy się nie mieszały
        lock_guard<mutex> lock(mtx_logi);
        cout << "[MONITOR] Temperatura systemu: 42 stopni C - Status: OK" << endl;
    }
}

// --- 2. LOGGER (Wątek z synchronizacją - "Consumer") ---
void loggers() {
    while (true) {
        unique_lock<mutex> lock(mtx_logi);
        
        // Czekamy, aż wpadnie log LUB dostaniemy sygnał do zakończenia
        cv_logi.wait(lock, [] { return !logi.empty() || czy_koniec; });

        // Wypisujemy wszystko, co aktualnie jest w kolejce
        while (!logi.empty()) {
            cout << "  >>> [LOGGER] " << logi.front() << endl;
            logi.pop();
        }

        // Jeśli main ustawił flagę końcową i kolejka jest już pusta, wychodzimy z pętli
        if (czy_koniec) break;
    }
}
// Funkcja pomocnicza do bezpiecznego dodawania logów z dowolnego miejsca
void dodajLog(string msg) {
    lock_guard<mutex> lock(mtx_logi);
    logi.push(msg);
    cv_logi.notify_one(); // POWIADOMIENIE: Budzimy Loggera, bo ma nową pracę!
}

// --- 3. ANALITYK (Wątek z wynikiem - "Worker") ---
void analityk(promise<int> obietnica, vector<int> dane) {
    dodajLog("Analityk rozpoczyna liczenie...");
    int suma = 0;
    for (int liczba : dane) {
        suma += liczba;
    }
    // Symulacja ciężkich obliczeń przez 3 sekundy
    this_thread::sleep_for(chrono::seconds(3));
    // Wysyłamy wynik do main przez obietnicę
    obietnica.set_value(suma);
}

// --- 4. GŁÓWNY WĄTEK (Szef - "Orchestrator") ---
int main() {
    cout << "--- START SYSTEMU CSO ---" << endl;
    // A. Startujemy Monitor i od razu go odłączamy
    thread t_monitor(miejski_monitoring);
    t_monitor.detach();
    // B. Startujemy Loggera (będzie czekał na powiadomienia)
    thread t_logger(loggers);
    dodajLog("System operacyjny zainicjalizowany");
    // C. Przygotowujemy zadanie dla Analityka
    vector<int> dane_do_analizy = {10, 20, 30, 40, 50};
    promise<int> obietnica_sumy;
    future<int> wynik_future = obietnica_sumy.get_future();
    // Uruchamiamy Analityka, przenosząc (move) obietnicę
    thread t_analityk(analityk, move(obietnica_sumy), dane_do_analizy);
    dodajLog("Zlecono zadanie analityczne");
    // D. Odbieramy wynik (main tutaj stanie i poczeka na set_value)
    int wynik_koncowy = wynik_future.get();
    // Ponieważ get() czeka na zakończenie pracy analityka, możemy go bezpiecznie dołączyć
    t_analityk.join();
    dodajLog("Otrzymano raport od Analityka. Wynik: " + to_string(wynik_koncowy));
    cout << "\n[MAIN] RAPORT FINALNY: " << wynik_koncowy << "\n" << endl;
    // E. Zamykamy system
    dodajLog("Rozpoczynam procedurę zamykania...");
    {
        lock_guard<mutex> lock(mtx_logi);
        czy_koniec = true;
    }
    cv_logi.notify_all(); // Ostatnie powiadomienie dla Loggera, żeby sprawdził czy_koniec
    t_logger.join(); // Czekamy, aż Logger wypisze ostatnie logi i skończy
    cout << "--- SYSTEM ZAMKNIETY ---" << endl;
    return 0;
}