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
mutex mtx;
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
        thread t(procesuj_dane,rozmiar/4*i,rozmiar/4*(i+1),i,ref(signals));
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