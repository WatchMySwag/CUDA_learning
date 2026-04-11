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
    scoped_lock(from.mtx, to.mtx); //scoped_lock jest nowym mechanizmem w C++17, który pozwala na jednoczesne zablokowanie wielu mutexów bez ryzyka deadlocka. W przeciwieństwie do lock_guard, który blokuje tylko jeden mutex, scoped_lock może blokować wiele mutexów naraz. Dzięki temu, nawet jeśli dwa wątki próbują zablokować te same mutexy w różnej kolejności, scoped_lock zapewnia, że nie dojdzie do zakleszczenia (deadlocka). To jest kluczowa różnica - scoped_lock jest zaprojektowany tak, aby unikać deadlocków, podczas gdy lock_guard tego nie gwarantuje.
    this_thread::sleep_for(chrono::milliseconds(100));
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