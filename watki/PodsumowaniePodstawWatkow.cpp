#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>
#include <future>
#include <condition_variable>
#include <latch>
#include <semaphore>
#include <barrier>
#include <syncstream>

using namespace std;
using namespace std::chrono_literals;

// --- ZASOBY GLOBALNE ---
mutex mtx_fuel;
condition_variable cv_fuel;
bool fuel_needed = false;
atomic<int> wystartowalo(0);

// --- 1. SYSTEMY WIEŻY (5 wątków) ---
void system_wiezy(latch& start_gate, string nazwa) {
    osyncstream(cout) << "[WIEŻA] Kalibracja systemu: " << nazwa << "..." << endl;
    this_thread::sleep_for(800ms);
    osyncstream(cout) << "[WIEŻA] System " << nazwa << " GOTOWY." << endl;
    start_gate.count_down(); // Każdy z 5 systemów odlicza raz
}

// --- 2. CYSTERNA (1 wątek tła) ---
void cysterna_worker(stop_token st) {
    osyncstream(cout) << "[CYSTERNA] System tankowania aktywny." << endl;
    while (!st.stop_requested()) {
        unique_lock lock(mtx_fuel);
        // Czekamy na sygnał od samolotu LUB na stop_token
        if (cv_fuel.wait_for(lock, 200ms, [&] { return fuel_needed || st.stop_requested(); })) {
            if (st.stop_requested()) break;
            
            osyncstream(cout) << "[CYSTERNA] Tankowanie zbiornika głównego po starcie..." << endl;
            this_thread::sleep_for(500ms);
            fuel_needed = false;
        }
    }
    osyncstream(cout) << "[CYSTERNA] System tankowania wyłączony." << endl;
}

// --- 3. SAMOLOT (10 wątków) ---
void samolot_worker(int id, latch& wieza, barrier<>& odprawa, counting_semaphore<>& pasy) {
    // KROK 1: Czekamy, aż Wieża skończy kalibrację (5 systemów)
    wieza.wait(); 

    // KROK 2: Odprawa (Bariera na 10 samolotów)
    osyncstream(cout) << "[SAMOLOT " << id << "] Sprawdzanie biletów..." << endl;
    odprawa.arrive_and_wait(); // Wszyscy muszą sprawdzić bilety, żeby ruszyć do bagażu
    
    osyncstream(cout) << "[SAMOLOT " << id << "] Kontrola bagażu..." << endl;
    odprawa.arrive_and_wait(); // Wszyscy muszą przejść bagaż, żeby ruszyć na pas

    // KROK 3: Start (Semafor na 2 pasy)
    {
        pasy.acquire();
        osyncstream(cout) << "[PAS] Samolot " << id << " STARTUJE..." << endl;
        this_thread::sleep_for(1s);
        osyncstream(cout) << "[PAS] Samolot " << id << " w powietrzu." << endl;
        wystartowalo++;
        pasy.release();
    }

    // KROK 4: Powiadomienie cysterny
    {
        lock_guard lock(mtx_fuel);
        fuel_needed = true;
    }
    cv_fuel.notify_all();
}

// --- MAIN (Orkiestracja) ---
int main() {
    const int LICZBA_SYSTEMOW = 5;
    const int LICZBA_SAMOLOTOW = 10;

    latch tower_latch(LICZBA_SYSTEMOW);
    barrier aircraft_barrier(LICZBA_SAMOLOTOW);
    counting_semaphore<2> runway_sem(2);

    // 1. Uruchamiamy cysternę w tle
    jthread cysterna(cysterna_worker);

    // 2. Uruchamiamy systemy wieży (5 wątków)
    vector<jthread> tower_threads;
    vector<string> systemy = {"Radar", "Radio", "Pogoda", "Światła", "IT"};
    for (auto& s : systemy) {
        tower_threads.emplace_back(system_wiezy, ref(tower_latch), s);
    }

    // 3. Uruchamiamy samoloty (10 wątków)
    vector<jthread> aircraft_threads;
    for (int i = 0; i < LICZBA_SAMOLOTOW; ++i) {
        aircraft_threads.emplace_back(samolot_worker, i + 1, ref(tower_latch), ref(aircraft_barrier), ref(runway_sem));
    }

    // 4. Czekamy, aż samoloty wystartują (inaczej raport będzie pusty)
    for (auto& t : aircraft_threads) {
        if (t.joinable()) t.join();
    }

    // 5. Raport asynchroniczny
    auto raport = async(launch::async, [] { return wystartowalo.load(); });
    cout << "\n=== RAPORT LOTNISKA ===\n";
    cout << "Udane starty: " << raport.get() << endl;

    // 6. Kończymy pracę (cysterna zostanie zatrzymana przez stop_token w destruktorze jthread)
    cout << "Zamykanie lotniska..." << endl;
    
    return 0;
}
//to do: cwiczyc wielewatkowosc