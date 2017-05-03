#include <iostream>
#include <cstdio>
#include <pthread.h>
#include <cstdlib>


using namespace std;



class Counter {

    long long value;
    public:
    Counter() {
        value = 0;
    }

    void Increment() {
        value++;
    }

    void Print() {
        cout << value << endl;
    }
};

Counter x;
pthread_mutex_t mlock;
int threadNum;

void *ThreadRunner(void* t) {
	//long long each = 300000000/threadNum;
    for (int k = 0; k < 100000000; ++k) {
        pthread_mutex_lock(&mlock);
        x.Increment();
        pthread_mutex_unlock(&mlock);
    }
    pthread_exit(NULL);
}


int main(int argc, char* argv[]) {

	if (argc < 2) {
		cout << "Not enough args ./mutex <Num of threads>" << endl;
		return 1;
	}	
	sscanf(argv[1], "%d", &threadNum);
    pthread_t tid[threadNum];
    if (pthread_mutex_init(&mlock, NULL)) {
        cout << "Pthread mutex init failed" << endl;
        return 1;
    }
    int returnCode;
    for (int i = 0; i < threadNum; ++i) {
        cout << "In main: Creating thread " << i << endl;
        returnCode = pthread_create(&tid[i], NULL, ThreadRunner, 0);
        if (returnCode) {
            cout << "ERROR: Return Code from pthread_create is " << returnCode << endl;
            exit(-1);
        }
    }
    for (int i = 0; i < threadNum; ++i) {
        pthread_join(tid[i], NULL);
    } 
    x.Print();
    pthread_mutex_destroy(&mlock);
    pthread_exit(NULL);
    return 0;
}
