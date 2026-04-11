#include<iostream>
#include <latch>
#include <thread>
#include <vector>
#include <semaphore>
#include <barrier>
#include <chrono>
#include <syncstream>

using namespace std;

counting_semaphore<2> sem(2);
void poczatek(latch& start, int id){
    osyncstream(cout)<<"Pracownik "<<id<< " czeka na start"<<endl;
    this_thread::sleep_for(chrono::seconds(1*id));
    osyncstream(cout)<<"Pracownik "<<id<<" gotowy"<<endl;
    start.arrive_and_wait(); // czekamy aż wszyscy pracownicy będą gotowi, czyli aż latch osiągnie 0
    osyncstream(cout)<<"Pracownik "<<id<<" startuje"<<endl;
}

void zadanie (int id, barrier<>& bar){
    osyncstream(cout)<<"Pracownik "<<id<<" napisal couting_semaphore"<<endl;
    bar.arrive_and_wait(); 
    osyncstream(cout)<<"Pracownik "<<id<<" napisal barrier"<<endl;
    bar.arrive_and_wait();
    osyncstream(cout)<<"Pracownik "<<id<<" napisal latch"<<endl;
    bar.arrive_and_wait();
}

void biuro(int id, latch& start, barrier<>& bar){
    poczatek(start, id);
    osyncstream(cout)<<"Pracownik "<<id<<" czeka na dostep do biura"<<endl;
    sem.acquire(); // czekamy aż będzie wolne miejsce w biurze, czyli aż semafor będzie miał wartość większą niż 0
    osyncstream(cout)<<"Pracownik "<<id<<" korzysta z biura"<<endl;
    this_thread::sleep_for(chrono::seconds(1)); 
    sem.release(); // zwalniamy miejsce w biurze
    osyncstream(cout)<<"Pracownik "<<id<<" zakończył pracę w biurze"<<endl;
    zadanie(id, bar);
}

void sprawdzacz(stop_token st, int id){
    osyncstream(cout)<<"Sprawdzacz "<<id<<"  sprzata"<<endl;
    while(!st.stop_requested()){
        this_thread::sleep_for(chrono::seconds(1));
    }
    osyncstream(cout)<<"Sprawdzacz "<<id<<"  konczy prace"<<endl;
}
int main(){
    latch start(3);
    barrier<> bar(3);
    vector<jthread> threads; //kolejnosc ma znaczenie, bo latch musi być zainicjalizowany przed wątkami, a bariera musi być zainicjalizowana przed wątkami, bo wątki będą z nich korzystać bo stos LIFO, więc najpierw musimy stworzyć latch i barierę, a potem wątki, które będą z nich korzystać.
    for(int i=0;i<3;i++){
        threads.push_back(jthread(biuro, i, ref(start), ref(bar)));
    }
    jthread sprawdzacz1(sprawdzacz, 1);
    jthread sprawdzacz2(sprawdzacz, 2);
    this_thread::sleep_for(chrono::seconds(3));
    sprawdzacz1.request_stop(); // wysyłamy sygnał do sprawdzacza, że ma przestać sprzątać
    this_thread::sleep_for(chrono::seconds(2));
    cout<<"Biuro zamknięte, sprawdzacze kończą pracę"<<endl;
    return 0;
}