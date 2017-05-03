#include <iostream>
#include <pthread.h>
#include <cstdio>
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
pthread_spinlock_t spinlock;
int threadNum;

void *ThreadRunner(void*) {
//	long long each = 300000000/threadNum;
    for (int k = 0; k < 100000000; ++k) {
        pthread_spin_lock(&spinlock);
        x.Increment();
        pthread_spin_unlock(&spinlock);
    }
    pthread_exit(NULL);
}


int main(int argc, char* argv[]) {

	if (argc < 2) { 
		cout << "Not enough args ./spinlock <Num of thread>" << endl;
		return 1;
	}
	sscanf(argv[1], "%d", &threadNum);
    pthread_spin_init(&spinlock, 0);
    pthread_t tid[threadNum];
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
    x.Print();

    pthread_exit(NULL);

    return 0;
}
