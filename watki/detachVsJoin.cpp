#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

using namespace std;

mutex mtx;
condition_variable cv;
bool koniecPracy = false;

void obliczenia(){
    int wynik = 0;
    for(int i=0;i<10;i++){
        wynik += i;
        cout<<"iteracja "<< i+1 <<": "<<wynik<<endl;
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    cout<<"Wynik: "<<wynik<<endl;
}
void monitor_systemu(){
        while(true){
        cout<<"Monitor systemu: wszystko działa poprawnie"<<endl;
        this_thread::sleep_for(chrono::seconds(1)); 
}
}
int main() { 
  thread monitor(monitor_systemu); 
  monitor.detach(); //konczy sie w momencie zakonczenia maina, taki deamon - watek, ktory dziala w tle
  thread obliczeniaThread(obliczenia);
  cout<<"Obliczenia wystartowały"<<endl;
    obliczeniaThread.join(); //dyrygent - organizuje prace watkow
    /*konczy sie w momencie zakonczenia obliczen, z joinem mamy pewnosc, ze obliczenia sie koncza zanim program sie zakonczy, a z detachem nie mamy tej pewnosci, bo main moze sie zakonczyc zanim 
    obliczenia sie skoncza, co prowadzi do nieprzewidywalnego zachowania programu. Dlatego w tym przypadku lepiej jest użyć join(), żeby mieć kontrolę nad zakończeniem wątku obliczeń. Dzięki temu mamy pewność, że 
    wszystkie obliczenia zostaną wykonane przed zakończeniem programu, a monitor systemu będzie działał równolegle i informował nas o stanie systemu podczas trwania obliczeń.*/
    cout<<"Obliczenia zakończyły się"<<endl;
}
 /*
 Twoja złota zasada:
    Jeśli wątek ma być usługą (np. sprawdzanie połączenia, muzyka, monitorowanie), dajesz mu detach.
    Jeśli wątek ma być zadaniem (np. oblicz sumę, pobierz plik, zapisz dane), dajesz mu join.
 */