#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#define THREAD_NUM 4

using namespace std;

class MonteCarlo{ 


    double inCircle;
    double inSquare;
    double minX;
    double minY;
    double maxX;
    double maxY;

    unsigned seed;
    public:
    MonteCarlo() {
        inCircle = 0;
        inSquare = 0;
        minX = 0;
        minY = 0;
        maxX = 0;
        maxY = 0;
        srand(time(NULL));
        seed = rand();
    } 

    void init(double x1, double x2, double y1, double y2) {
        minX = x1;
        minY = y1;
        maxX = x2;
        maxY = y2;
    }

    void generate() {
        double x = double(rand_r(&seed))/RAND_MAX * (maxX - minX) + minX;
        double y = double(rand_r(&seed))/RAND_MAX * (maxY - minY) + minY;
        inSquare++;
        if (x*x+y*y <= 1) {
            inCircle++;
        }
    } 
    
    double getInCircle() {
        return inCircle;
    }
    double getInSquare() {
        return inSquare;
    }

};

MonteCarlo x[THREAD_NUM];

void* ThreadRunner(void* args) {

    int id = (*(pair<int, int>*)args).first;
    int each = (*(pair<int, int>*)args).second;
    for (int i = 0; i < each; ++i) {
        x[id].generate();
    }

    pthread_exit(NULL);
}

int main (int argc, char* argv[]){
    if (argc < 2) {
        cout << "Enter N (./PIFree <N>)" << endl;
        return 0;
    }
    long long N;
    sscanf(argv[1], "%lld", &N);
    pthread_t tid[THREAD_NUM];
    int returnCode;
    double dx[4] = {-1,0,-1,0};
    double dy[4] = {-1,-1,0,0};
    pair<int, int> args[4];
    for (int i = 0; i < THREAD_NUM; ++i) {
        args[i].first = i;
        args[i].second = N / THREAD_NUM;
        x[i].init(dx[i], dx[i]+1, dy[i], dy[i]+1);
        cout << "In main: Creating thread " << i << endl;
        returnCode = pthread_create(&tid[i], NULL, ThreadRunner, (void*) &args[i]);
        if (returnCode) {
            cout << "ERROR: Return Code from pthread_create is " << returnCode << endl;
            exit(-1);
        }
    }
    for (int i = 0; i < THREAD_NUM; ++i) {
        pthread_join(tid[i], NULL);
    }

    double inCircle = 0;
    double inSquare = 0;

    for (int i = 0; i < THREAD_NUM; ++i) {
        inCircle += x[i].getInCircle();
        inSquare += x[i].getInSquare();
    }

    cout << 4 * inCircle / inSquare << endl;

    pthread_exit(NULL);
    return 0;
}
