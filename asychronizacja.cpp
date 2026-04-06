#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <future>
#include <condition_variable>
using namespace std;

int pobierzDaneZInternetu(string url){
    cout<<"Pobieranie danych z "<<url<<"..."<<endl;
    this_thread::sleep_for(chrono::seconds(3)); // symulacja czasu pobierania
    cout<<url.size()<<" bajtów danych pobrano z "<<url<<"."<<endl;
    return url.size();
}
int main() {
    future<int> wynik = async(launch::async, pobierzDaneZInternetu, "http://example.com/dane");
    future<int> wynik2 = async(launch::async, pobierzDaneZInternetu, "http://example.com/inne_dane");
    cout<<"Pobieranie danych rozpoczęte..."<<endl;
    int rozmiar1 = wynik.get(); // get() czeka na wynik z pobierzDaneZInternetu i zwraca go
    int rozmiar2 = wynik2.get();
    cout<<"Wszystkie dane zostały pobrane."<<endl;
    cout<<"Suma rozmiarów danych: "<<rozmiar1 + rozmiar2<<" bajtów."<<endl;
    return 0;
}