#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <time.h>
#include <stdio.h>

using namespace std;

const int MAX_WEIGHT = 10;
const int WEIGHT_THRESHOLD = 5;
const int ESTEIRA1_INTERVAL = 2;
const int ESTEIRA2_INTERVAL = 1;
const int DISPLAY_UPDATE_INTERVAL = 2;
const int WEIGHT_CHECK_INTERVAL = 5;
bool parada = true;
atomic<int> esteira1_count(0);
atomic<int> esteira2_count(0);
atomic<int> total_weight(0);

mutex display_mutex;
mutex weight_mutex;
condition_variable weight_check_cv;

double time1, timedif;

void generate_weight(int& weight) {
    weight = (rand() % MAX_WEIGHT) + 1;
}

void esteira1_worker() {
    while (true) {
        int weight;
        generate_weight(weight);

        if (weight >= WEIGHT_THRESHOLD) {
            this_thread::sleep_for(chrono::seconds(ESTEIRA1_INTERVAL));

            unique_lock<mutex> lock(weight_mutex);
            total_weight += weight;
            esteira1_count++;
            if ((esteira1_count + esteira2_count) % WEIGHT_CHECK_INTERVAL == 0) {
                weight_check_cv.notify_one();
            }
        }
        else {
            this_thread::sleep_for(chrono::seconds(ESTEIRA2_INTERVAL));
        }


        unique_lock<mutex> lock(display_mutex);
        cout << "Esteira 1 - peso: " << weight << ", contagem: " << esteira1_count << endl;
    }
}

void esteira2_worker() {
    while (true) {
        int weight;
        generate_weight(weight);

        this_thread::sleep_for(chrono::seconds(ESTEIRA2_INTERVAL));

        unique_lock<mutex> lock(weight_mutex);
        total_weight += weight;
        esteira2_count++;
        if ((esteira1_count + esteira2_count) % WEIGHT_CHECK_INTERVAL == 0) {
            weight_check_cv.notify_one();
        }

        unique_lock<mutex> locka(display_mutex);
        cout << "Esteira 2 - peso: " << weight << ", contagem: " << esteira2_count << endl;
    }
}



int main() {
    srand(time(NULL));

    time1 = (double)clock();
    time1 = time1 / CLOCKS_PER_SEC;

    thread esteira1_thread(esteira1_worker);
    thread esteira2_thread(esteira2_worker);


    while (true) {

        if ((esteira1_count + esteira2_count) % 10 == 0 && esteira1_count + esteira2_count != 0) {

            unique_lock<mutex> lock(display_mutex);
            cout << "Peso total processado: " << total_weight << " Quantidade itens: " << esteira1_count + esteira2_count << endl;
            this_thread::sleep_for(chrono::seconds(1));

        }

        if (esteira1_count + esteira2_count >= 100) { //Criterio de parada para a obtencao de resultado de testes
            timedif = (((double)clock()) / CLOCKS_PER_SEC) - time1;
            cout << "Tempo Total " << timedif << endl;
            abort();
        }

    }

    esteira1_thread.join();
    esteira2_thread.join();




    return 0;
}