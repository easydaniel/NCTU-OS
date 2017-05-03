#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define THREAD_NUM 4

using namespace std;

pthread_mutex_t mlock;



class MonteCarlo{ 


    double inCircle;
    double inSquare;
    unsigned seed;
    public:
    MonteCarlo() {
        inCircle = 0;
        inSquare = 0;
        srand(time(NULL));
        seed = rand();
    } 
    void generate() {
        double x = double(rand_r(&seed))/RAND_MAX;
        double y = double(rand_r(&seed))/RAND_MAX;
        pthread_mutex_lock(&mlock);
        inSquare++;
        if (x*x+y*y <= 1) {
            inCircle++;
        }
        pthread_mutex_unlock(&mlock);
    } 

    double estimate() {
        return 4 * inCircle / inSquare;
    }
};

MonteCarlo x;

void* ThreadRunner(void* args) {

    int N = *(int*) args;
    for (int i = 0; i < N; ++i) {
        x.generate();
    }
    pthread_exit(NULL);
}

int main (int argc, char* argv[]){
    if (argc < 2) {
        cout << "Enter N (./PILock <N>)" << endl;
        return 0;
    }
    long long N;
    sscanf(argv[1], "%lld", &N);
    pthread_t tid[THREAD_NUM];
    if (pthread_mutex_init(&mlock, NULL)) {
        cout << "Pthread mutex init failed" << endl;
        return 1;
    }
    int returnCode;
    for (int i = 0; i < THREAD_NUM; ++i) {
        cout << "In main: Creating thread " << i << endl;
        returnCode = pthread_create(&tid[i], NULL, ThreadRunner, (void*) &N);
        if (returnCode) {
            cout << "ERROR: Return Code from pthread_create is " << returnCode << endl;
            exit(-1);
        }
    }
    for (int i = 0; i < THREAD_NUM; ++i) {
        pthread_join(tid[i], NULL);
    }
    pthread_mutex_destroy(&mlock);

    cout << x.estimate() << endl;

    pthread_exit(NULL);
    return 0;
}
