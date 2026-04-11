#include <iostream>
#include <thread>
#include <vector>   // DODANO
#include <mutex>    // DODANO
#include <atomic>   // DODANO
#include <cmath>   
#include <chrono>  // DODANO
#define rozmiar 1000000
using namespace std;
vector<double> signals(rozmiar);
atomic<int> signal_count(0);
vector<thread> threads;
vector<string> raport;
mutex mtx; //musi byc globalny, bo inaczej kazdy watek mialby swoj mutex, a my chcemy zeby wszystkie watki korzystaly z tego samego mutexa, zeby nie bylo problemow z synchronizacja
void procesuj_dane(int start, int end, int id, vector<double>& dane) {
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int i = start; i < end; ++i) {
        dane[i] = sin(dane[i]) + cos(dane[i]);
        if(i % 1000 == 0) {
            signal_count.fetch_add(1000);
        }
    }
   auto end_time = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        lock_guard<mutex> lock(mtx);
        raport.push_back("Watek " + to_string(id) + " przetworzyl dane od " + to_string(start) + " do " + to_string(end) + " w czasie " + to_string(duration.count()) + " ms");
}
int main() {
    for(int i = 0; i < rozmiar; ++i) {
        signals[i] = rand() / (double)RAND_MAX;
    }
    for(int i=0;i<4;i++){
        thread t(procesuj_dane,rozmiar/4*i,rozmiar/4*(i+1),i,ref(signals)); //musi byc ref, bo inaczej przekazywaloby kopie wektora, a my chcemy przekazac referencje
        threads.push_back(move(t)); //musi byc move, bo thread nie jest kopiowalny
    }
    for(auto& t : threads) {
        t.join();
    }
    for(const auto& r : raport) {
        cout << r << endl;
    }
    cout << "Liczba przetworzonych sygnalow: " << signal_count.load() << endl;        
    return 0;
}
/*


woje zadanie (Wymagania techniczne):
1. Przygotowanie danych:
    Stwórz std::vector<double> i wypełnij go 10 000 000 losowych liczb.
    Stwórz zmienną std::atomic<int> progress{0}, która będzie zliczać, ile liczb już przetworzono.
2. Podział pracy (Klucz do wydajności):
    Nie twórz wątku dla każdej liczby (to by zabiło system!).
    Podziel tablicę na 4 lub 8 dużych części (tzw. "chunki"). Jeśli masz 10 mln liczb i 4 wątki, każdy dostaje 2,5 mln liczb do obrobienia.
    Każdy wątek powinien dostać indeks początkowy i końcowy swojej części.
3. Zadanie dla wątku:
    Każdy wątek w pętli przechodzi przez swój zakres i wykonuje "ciężkie" obliczenie: data[i] = std::sqrt(std::sin(data[i]) * std::cos(data[i]) + 1.0).
    Po zakończeniu swojej części, wątek powinien użyć mutexa, aby dopisać do wspólnego std::vector<string> raporty informację: "Wątek X skończył pracę w czasie Y ms".
4. Statystyki (Atomic):
    Wewnątrz pętli obliczeniowej, co 1000 operacji, wątek powinien zwiększać atomowy licznik progress += 1000.
    W main(), podczas gdy wątki pracują, stwórz dodatkowy wątek "Monitorujący", który co 100ms wypisuje na ekran: "Postęp: X / 10 000 000".
*/