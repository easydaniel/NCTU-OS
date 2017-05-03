#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <pthread.h>

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
int spinlock;
int threadNum;

void homemade_spin_lock(int* spinlock_addr) {
    asm(
            "spin_lock: \n\t"
            "xorl %%ecx, %%ecx \n\t"
            "incl %%ecx \n\t"
            "spin_lock_retry: \n\t"
            "xorl %%eax, %%eax \n\t"
            "lock; cmpxchgl %%ecx, (%0) \n\t" "jnz spin_lock_retry \n\t"
            : : "r" (spinlock_addr) : "ecx", "eax" );
}

void homemade_spin_unlock(int* spinlock_addr) {
    asm(
            "spin_unlock: \n\t" "movl $0, (%0) \n\t"
            : : "r" (spinlock_addr) : );
}

void *ThreadRunner(void*) {
//	long long each = 300000000/threadNum;
    for (int k = 0; k < 100000000; ++k) {
        homemade_spin_lock(&spinlock);
        x.Increment();
        homemade_spin_unlock(&spinlock);
    }
    pthread_exit(NULL);
}


int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "Not enough args ./homemade_spinlock <Num of thread>" << endl;
		return 1;
	}
	sscanf(argv[1], "%d", &threadNum);
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
