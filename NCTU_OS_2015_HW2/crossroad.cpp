#include <iostream>
#include <pthread.h>
#include <cstdio>
#define Nid 0
#define Eid 1
#define Wid 2
#define Sid 3
using namespace std;

int timer[4];
int hasLock[4];
bool done[4];
bool unDeadlock;
pthread_mutex_t check;
pthread_mutex_t doneCheck[4];
pthread_mutex_t timerLock[4];
pthread_mutex_t roadLock[4];


void init() {
    for (int i = 0; i < 4; ++i) {
        timer[i] = 1;
        hasLock[i] = 0;
        done[i] = false;
        unDeadlock = false;
        pthread_mutex_init(&check, NULL);
        pthread_mutex_init(&doneCheck[i], NULL);
        pthread_mutex_init(&timerLock[i], NULL);
        pthread_mutex_init(&roadLock[i], NULL);
    }
}

int getTimer(int id) {
    pthread_mutex_lock(&timerLock[id]);
    int retVal = timer[id];
    pthread_mutex_unlock(&timerLock[id]);
    return retVal;
}

bool deadlock() {
    pthread_mutex_lock(&check);
    bool re = true;
    for (int i = 0; i < 4; ++i) {
        re &= hasLock[i];
    }
    pthread_mutex_unlock(&check);
    return re;
}

int countLock(int id) {
    pthread_mutex_lock(&check);
    int re = hasLock[id];
    pthread_mutex_unlock(&check);
    return re;
}

void addLockCount(int id) {
    pthread_mutex_lock(&check);
    hasLock[id]++;
    pthread_mutex_unlock(&check);
}

void resetCountLock(int id) {
    pthread_mutex_lock(&check);
    hasLock[id] = 0;
    pthread_mutex_unlock(&check);
}

void addTimer(int id) {
    pthread_mutex_lock(&timerLock[id]);
    timer[id]++;
    pthread_mutex_unlock(&timerLock[id]);
}

void setDone(int id) {
    pthread_mutex_lock(&doneCheck[id]);
    done[id] = true;
    pthread_mutex_unlock(&doneCheck[id]);
}

bool compare(int me, int other) {
    pthread_mutex_lock(&doneCheck[other]);
    bool _done = done[other];
    pthread_mutex_unlock(&doneCheck[other]);
    return (_done or getTimer(me) <= getTimer(other));
}

void *ThreadN(void* args) {
    int carNum = 0;
    while (carNum < *(int*) args) {
        if (compare(Nid, Eid) and compare(Nid, Wid) and compare(Nid, Sid)) {
            if (countLock(Nid) == 2) {
                //cout << "N " << carNum << " leaves at " << step() << endl;
                printf("N %d leaves at %d\n", carNum, getTimer(Nid));
                pthread_mutex_unlock(&roadLock[Nid]); 
                pthread_mutex_unlock(&roadLock[Wid]); 
                carNum++;
                resetCountLock(Nid);
            } else if (countLock(Nid) == 1) {
                if (not unDeadlock and pthread_mutex_trylock(&roadLock[Wid])) {
                    //cout << "N acquire lock W fail" << endl; 
                    if (deadlock()) {
                        //cout << "A DEADLOCK HAPPENS AT " << step() << endl;
                        printf("A DEADLOCK HAPPENS AT %d\n", getTimer(Nid));
                        pthread_mutex_lock(&check);
                        unDeadlock = true;
                        pthread_mutex_unlock(&check);
                        pthread_mutex_unlock(&roadLock[Nid]);
                    }
                } else if (not unDeadlock) {
                    //cout << "N acquire lock W success" << endl; 
                    addLockCount(Nid);
                }
            } else {
                if(pthread_mutex_trylock(&roadLock[Nid])) {
                    //cout << "N acquire lock N fail" << endl; 
                } else {
                    //cout << "N acquire lock N success" << endl; 
                    addLockCount(Nid);
                }
            }
            addTimer(Nid);
        }
    }
    setDone(Nid);
    pthread_exit(NULL);
}
void *ThreadE(void* args) {
    int carNum = 0;
    while (carNum < *(int*) args) {
        if (compare(Eid, Nid) and compare(Eid, Wid) and compare(Eid, Sid)) {
            if (countLock(Eid) == 2) {
                //cout << "E " << carNum << " leaves at " << step() << endl;
                printf("E %d leaves at %d\n", carNum, getTimer(Eid));
                pthread_mutex_unlock(&roadLock[Eid]); 
                pthread_mutex_unlock(&roadLock[Nid]); 

                pthread_mutex_lock(&check);
                unDeadlock = false;
                pthread_mutex_unlock(&check);
                carNum++;
                resetCountLock(Eid);
            } else if (countLock(Eid) == 1) {
                if(pthread_mutex_trylock(&roadLock[Nid])) {
                    //cout << "E acquire lock N fail" << endl; 
                } else {
                    //cout << "E acquire lock N success" << endl; 
                    addLockCount(Eid);
                }
            } else {
                if (pthread_mutex_trylock(&roadLock[Eid])) {
                    //cout << "E acquire lock E fail" << endl;
                } else {
                    //cout << "E acquire lock E sucess" << endl;
                    addLockCount(Eid);
                }
            }
            addTimer(Eid);
        }
    }
    setDone(Eid);
    pthread_exit(NULL);
}
void *ThreadS(void* args) {
    int carNum = 0;
    while (carNum < *(int*) args) {
        if (compare(Sid, Nid) and compare(Sid, Wid) and compare(Sid, Eid)) {
            if (countLock(Sid) == 2) {
                //cout << "S " << carNum << " leaves at " << step() << endl;
                printf("S %d leaves at %d\n", carNum, getTimer(Sid));
                pthread_mutex_unlock(&roadLock[Sid]); 
                pthread_mutex_unlock(&roadLock[Eid]); 
                carNum++;
                resetCountLock(Sid);
            } else if (countLock(Sid) == 1) {
                if (pthread_mutex_trylock(&roadLock[Eid])) {
                    //cout << "S acquire lock E fail" << endl; 
                } else {
                    //cout << "S acquire lock E success" << endl; 
                    addLockCount(Sid);
                }
            } else {
                if (pthread_mutex_trylock(&roadLock[Sid])) {
                    //cout << "S acquire lock S fail" << endl; 
                } else { 
                    //cout << "S acquire lock S success" << endl; 
                    addLockCount(Sid);
                }
            }
            addTimer(Sid);
        }
    }
    setDone(Sid);
    pthread_exit(NULL);
}
void *ThreadW(void* args) {
    int carNum = 0;
    while (carNum < *(int*) args) {
        if (compare(Wid, Nid) and compare(Wid, Sid) and compare(Wid, Eid)) {
            if (countLock(Wid) == 2) {
                //cout << "W " << carNum << " leaves at " << step() << endl;
                printf("W %d leaves at %d\n", carNum, getTimer(Wid));
                pthread_mutex_unlock(&roadLock[Wid]); 
                pthread_mutex_unlock(&roadLock[Sid]); 
                carNum++;
                resetCountLock(Wid);
            } else if (countLock(Wid) == 1) {
                if (pthread_mutex_trylock(&roadLock[Sid])) {
                    //cout << "W acquire lock S fail" << endl; 
                } else {
                    //cout << "W acquire lock S success" << endl; 
                    addLockCount(Wid);
                }
            } else {
                if(pthread_mutex_trylock(&roadLock[Wid])) {
                    //cout << "W acquire lock W fail" << tmp <<endl; 
                } else {
                    //cout << "W acquire lock W success" << endl; 
                    addLockCount(Wid);
                }
            }
            addTimer(Wid);
        }
    }
    setDone(Wid);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {

    if (argc < 5) {
        cout << "Not enough args ./cross N E S W" << endl;
        return 1;
    }
    int n, e, s, w;
    sscanf(argv[1], "%d", &n);
    sscanf(argv[2], "%d", &e);
    sscanf(argv[3], "%d", &s);
    sscanf(argv[4], "%d", &w);

    init();

    pthread_t NThread;
    pthread_t EThread;
    pthread_t WThread;
    pthread_t SThread;

    pthread_create(&NThread, NULL, ThreadN, (void*) &n);
    pthread_create(&EThread, NULL, ThreadE, (void*) &e);
    pthread_create(&WThread, NULL, ThreadW, (void*) &w);
    pthread_create(&SThread, NULL, ThreadS, (void*) &s);

    pthread_join(NThread, NULL);
    pthread_join(EThread, NULL);
    pthread_join(WThread, NULL);
    pthread_join(SThread, NULL);

    return 0;
}
