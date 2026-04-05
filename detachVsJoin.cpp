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
  monitor.detach();
  thread obliczeniaThread(obliczenia);
  cout<<"Obliczenia wystartowały"<<endl;
    obliczeniaThread.join();
    cout<<"Obliczenia zakończyły się"<<endl;
}
 