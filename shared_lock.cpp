#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <chrono>
using namespace std;

vector<string>ogloszenia = {"Witam i o zdrowie pytam"};
shared_mutex mtx;
mutex mtx_cout;
void czytelnik(int id) {
    for(int i=0; i<3; i++) {
        this_thread::sleep_for(chrono::milliseconds(100)); //wazne, zeby czytelnicy nie czytali w tym samym czasie, bo wtedy nie byloby widoczne dzialanie shared_locka, a tak mamy pewnosc, ze czytelnicy beda sie blokowac na pisarzu, a nie na sobie nawzajem
        shared_lock<shared_mutex> lock(mtx); // w forze by dzialy rozne watki, a nie jeden watek czytal trzy razy z rzedu 
        lock_guard<mutex> lockwyj(mtx_cout); //blokuje couta bo watki za bardzo rozrabiaja
        cout << "--- Czytelnik " << id << " zaczyna raport ---" << endl;
        for(int j=0; j<ogloszenia.size(); j++) {
            cout << "Czytelnik " << id << " czyta: " << ogloszenia[j] << endl;
        }
        cout << "--- Czytelnik " << id << " skonczyl ---" << endl;
    }
}

void pisarz(){
    unique_lock<shared_mutex> lock(mtx);
    ogloszenia.push_back("Dodaje informacje!!");
    this_thread::sleep_for(chrono::milliseconds(1000));
    cout<<"Pisarz dodał ogloszenie"<<endl;
}
int main(){
    vector<thread> czytelnikThread;
    for(int i=0;i<5;i++){
        czytelnikThread.push_back(thread(czytelnik,i+1));
    }
    this_thread::sleep_for(chrono::milliseconds(100));
    thread pisarzThread(pisarz);
    pisarzThread.join();
    this_thread::sleep_for(chrono::milliseconds(1000));
    for(int i=0;i<5;i++){
        czytelnikThread[i].join();
    }
    return 0;
}