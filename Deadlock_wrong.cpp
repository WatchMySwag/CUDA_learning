// Deadlock (zakleszczenie) to sytuacja patowa, w której dwa (lub więcej) wątki czekają na siebie nawzajem w nieskończoność.
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;
struct Account{
    int balance;
    mutex mtx; //zeby nie blokowac calego banku, tylko konta
    string name;
    Account(int b, string n) : balance(b), name(n) {} //konstruktor
};

void transfer(Account& from, Account& to, int amount){
    lock_guard<mutex>lock1(from.mtx);
    this_thread::sleep_for(chrono::milliseconds(100));
    lock_guard<mutex>lock2(to.mtx);
    if(amount > from.balance){
        cout<<"Zbyt duza srdokow\n";
        return;
    }
    else{
        from.balance -= amount;
        to.balance += amount;
        cout<<"Przelew z "<<from.name<<" do "<<to.name<<" na kwote "<<amount<<" zakonczony sukcesem\n";
    }
}

int main(){
    Account a{4000, "Antoni"};
    Account b{3000, "Barbara"};
    thread t1(transfer, ref(a), ref(b), 1000);
    thread t2(transfer, ref(b), ref(a), 500);
    t1.join();
    t2.join();
    cout<<"Stan konta "<<a.name<<": "<<a.balance<<endl;
    cout<<"Stan konta "<<b.name<<": "<<b.balance<<endl;
    return 0;
}
/*
Sekwencja "Uścisku Śmierci":

    Wątek t1 wchodzi do transfer (Antoni -> Barbara). Blokuje a.mtx.

    Wątek t2 wchodzi do transfer (Barbara -> Antoni). Blokuje b.mtx.

    Oba wątki idą spać na 100ms (sleep_for).

    Wątek t1 budzi się i mówi: „Dobra, teraz daj mi zamek Barbary (b.mtx)”. Ale Barbara jest już zajęta przez t2. t1 staje w kolejce.

    Wątek t2 budzi się i mówi: „Dobra, teraz daj mi zamek Antoniego (a.mtx)”. Ale Antoni jest zajęty przez t1. t2 staje w kolejce.

Obaj stoją. Obaj czekają. Nikt nie puści pierwszy. Mamy Deadlock.


*/