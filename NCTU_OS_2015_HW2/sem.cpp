#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <cstdio>
#include <semaphore.h>


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
sem_t sem;
int threadNum;

void *ThreadRunner(void*) {
//	long long each = 300000000/threadNum;
    for (int k = 0; k < 100000000; ++k) {
        sem_wait(&sem);
        x.Increment();
        sem_post(&sem);
    }
    pthread_exit(NULL);
}


int main(int argc, char* argv[]) {

	if (argc < 2) {
		cout << "Not enough args ./sem <Num of threads>" << endl;
		return 1;
	}
	sscanf(argv[1], "%d", &threadNum);
    pthread_t tid[threadNum];
    sem_init(&sem, 0, 1);
    int returnCode;
    for (int i = 0; i < threadNum; ++i) {
        cout << "In main: Creating thread " << i << endl;
        returnCode = pthread_create(&tid[i], NULL, ThreadRunner, NULL);
        if (returnCode) {
            cout << "ERROR: Return Code from pthread_create is " << returnCode << endl;
            exit(-1);
        }
    }
    for (int i = 0; i < threadNum; ++i) {
        pthread_join(tid[i], NULL);
    } 
    sem_destroy(&sem);
    x.Print();
    pthread_exit(NULL);
    return 0;
}
