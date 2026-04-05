#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <future>
#include <utility>
using namespace std;

void sprawdzaczHasla(promise<string>obietnica, string wpisaneHaslo){
    this_thread::sleep_for(chrono::seconds(2));
    if(wpisaneHaslo == "tajnehaslo"){
        obietnica.set_value("Haslo poprawne");
    } else {
        obietnica.set_value("Haslo niepoprawne");
    }
}

int main() { 
 string haslo;
 cout<<"Podaj haslo: ";
    cin>>haslo;
    promise<string> obietnica;
    future<string> przyszlosc = obietnica.get_future(); // tutaj tworzymy future, który będzie czekał na wynik z promise.
    thread sprawdzacz(sprawdzaczHasla,move(obietnica), haslo);
    cout<<"Sprawdzanie hasla..."<<endl;
    cout<<przyszlosc.get()<<endl; //get() czeka na wynik
    sprawdzacz.join();
}
