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
queue<int> zamówienia;

void Kelner() {
    while (true) {
        unique_lock<mutex> lock(mtx);// unique_lock, bo będziemy go odblokowywać przed cout, żeby nie blokować kucharza podczas pisania
        
        cv.wait(lock, [] { return !zamówienia.empty() || koniecPracy; });//tu lambda sprawdza, czy są zamówienia lub czy kuchnia skończyła pracę. Jeśli nie, kelner będzie czekał. Jeśli tak, kelner się obudzi i sprawdzi co dalej. To jest klucz do efektywności - kelner nie będzie ciągle sprawdzał kolejki (polling), tylko będzie spał aż coś się wydarzy (signaling).
            //w cv.wait kelner dopisuje sie do kolejki oczekujaych watkow, dlatego wywolanie w mainie cv.notify_one() budzi tylko jednego kelnera, a cv.notify_all() budzi wszystkich kelnerów (gdyby było ich więcej). To jest klucz do poprawnej synchronizacji - bez tego kelner mógłby się obudzić i sprawdzić kolejkę zanim kuchnia zdążyłaby dodać zamówienie, co prowadziłoby do błędów i nieefektywności. Dzięki condition_variable mamy pewność, że kelner będzie reagował tylko wtedy, gdy jest coś do zrobienia.
        if (koniecPracy && zamówienia.empty()) {
            break; 
        }

        // Przetwarzamy zamówienie (jeśli kolejka nie jest pusta)
        if (!zamówienia.empty()) {
            int zamówienie = zamówienia.front();
            zamówienia.pop();
            
            // Odblokowujemy przed cout, żeby nie blokować kucharza podczas pisania
            lock.unlock(); // odblokowujemy mutex, żeby kucharz mógł przyjmować kolejne zamówienia podczas gdy kelner pisze
            cout << "Kelner przyjal zamowienie: " << zamówienie << endl;
        }
    }
    cout << "Kelner idzie do domu." << endl;
}

int main() { //main jest kucharzem - watkiem, który przyjmuje zamówienia od klientów i przekazuje je kelnerowi
    thread kelner(Kelner); //dodatkowy wątek dla kelnera

    for (int i = 0; i < 10; i++) {
        {
            lock_guard<mutex> lock(mtx); // tutaj mutex bo zamówienia są współdzielone między wątkami i musimy zapewnić, że tylko jeden wątek na raz będzie modyfikował kolejkę. To jest klucz do bezpieczeństwa danych.
            zamówienia.push(i + 1);
            cout << "Klient zlozyl zamowienie nr: " << i + 1 << endl;
        } // Tu lock_guard puszcza mutex

        cv.notify_one(); // budzimy kelenera
        this_thread::sleep_for(chrono::milliseconds(200)); // Czas między klientami
    }

    // Kuchnia kończy pracę
    {
        lock_guard<mutex> lock(mtx); // mutex, bo modyfikujemy flagę koniecPracy, która jest współdzielona między wątkami
        koniecPracy = true;
    }
    cv.notify_all(); // Budzimy kelnera ostatni raz, żeby sprawdził flagę

    // KLUCZOWY MOMENT: Czekamy, aż kelner skończy pracę przed zamknięciem programu
    if (kelner.joinable()) { 
        kelner.join(); //join() czeka na reszte watkow, żeby się zakończyły przed kontynuowaniem. To jest klucz do poprawnego działania programu - bez tego main() mogłoby się zakończyć zanim kelner zdążyłby przetworzyć wszystkie zamówienia, co prowadziłoby do błędów i nieprzewidywalnego zachowania. Dzięki join() mamy pewność, że kelner skończy swoją pracę zanim program się zakończy.
    } 
    cout << "Restauracja zamknieta." << endl;
    return 0;
}
    /* kiedy uzywamy joinable?
- Kiedy chcemy sprawdzić, czy wątek jest nadal aktywny i można do niego dołączyć.
- Kiedy chcemy uniknąć błędu dołączenia do wątku, który już się zakończył.
- Kiedy chcemy mieć pewność, że wątek został poprawnie dołączony
przyklad:
vector<thread> watki(5);
watki[0] = thread(Kelner);
for(auto& t : watki) {
    if(t.joinable()) {
        t.join();
    }
}
W tym przykładzie sprawdzamy, czy każdy wątek jest joinable przed próbą dołączenia do niego, co zapobiega błędom i zapewnia poprawne działanie
    */