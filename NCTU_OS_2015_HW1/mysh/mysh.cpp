#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <termios.h>
#include <errno.h>
#include <iostream>
#include <cstring>
#include <string>
#include <csignal>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <set>

#define DEFAULT "\033[0;0m"
#define BLACK "\033[0;30m"
#define BLUE "\033[0;34m"
#define GREEN "\033[0;32m"
#define CYAN "\033[0;36m"
#define RED "\033[0;31m"
#define MAGENTA "\033[0;35m"
#define BROWN "\033[0;33m"

using namespace std;

bool backgroundCMD;
bool singleCMDFlag = false;
char pipeChar;
char spaceChar;
char homePath[128];
int parentPID;

set<int> processTABLE;






vector<string> parseSingleCMD(string cmd) {
    vector<string> res;
    int len = int(cmd.size());
    string buf;
    for (int i = 0; i < len; ++i) {
        if (cmd[i] == spaceChar and not buf.empty()) {
            res.push_back(buf);
            buf.clear();
        } else {
            buf += cmd[i];
        }
    }
    if (not buf.empty()) {
        res.push_back(buf);
        buf.clear();
    }
    return res;
}

void execute(vector<string> singleCMD) {
    string cmd = singleCMD.front();
    char** args = new char*[int(singleCMD.size()) + 1];
    for (int i = 0; i < int(singleCMD.size()); ++i) {
        args[i] = new char[int(singleCMD[i].size())];
        strcpy(args[i],singleCMD[i].c_str());
    }
    args[int(singleCMD.size())] = NULL;
    int childValue = execvp(args[0], args);
    if (childValue == -1) {
        for (int i = 0; i < int(singleCMD.size()); ++i) {
            delete [] args[i];
        }
        delete [] args;
        cout << CYAN << "-mysh: " << cmd << ": command not found" << DEFAULT << endl;
        exit(1);
    }
    exit(0);
}

vector<string> parseMultiCMD(char* cmd) {
    vector<string> res;
    int len = strlen(cmd);
    if (cmd[len-1] == '&') {
        backgroundCMD = true;
    }
    string buf;
    for (int i = 0; i < len; ++i) {
        if (cmd[i] == pipeChar and not buf.empty()) {
            res.push_back(buf);
            buf.clear();
        } else if (not buf.empty() and cmd[i] != '&') {
            buf += cmd[i];
        } else if (cmd[i] != ' ') {
            buf += cmd[i];
        }
    }
    if (not buf.empty()) {
        res.push_back(buf);
        buf.clear();
    }
    return res;
}


bool userPrompt() {
    char user[64],dir[128];
    // Get username
    getlogin_r(user, sizeof(user));
    // Get current working dir
    getcwd(dir, sizeof(dir));
    // Prompt
    cout << BLUE << "[" << user << "]" << DEFAULT << " in " << GREEN << "[" << dir << "]"<< DEFAULT << endl;
    cout << "mysh > ";
    return true;
}

void executeSingleCMD(vector<string> singleCMD) {
    // Get command
    string cmd = singleCMD.front();
    if (cmd == "exit") {
        int status;
        for (set<int>::iterator it = processTABLE.begin(); it != processTABLE.end(); ++it) {
            kill(*it, SIGINT);
            waitpid(*it, &status, 0);
        }
        cout << endl << "Goodbye!" << endl;
        exit(0);

    } else if (cmd == "cd") {
        string dir;
        int len = int(singleCMD.size());
        for (int i = 1; i < len; ++i) {
            dir += singleCMD[i];
        }
        if (dir.size() == 0 or dir[0] == '~') {
            dir = homePath;
        }
        if (chdir(dir.c_str())) {
            cout << RED << "-mysh: cd " << dir << " : No such file or directory" << DEFAULT << endl;
        }
    } else if (cmd == "fg") {
        if (singleCMD.size() == 2) {
            int pid = stoi(singleCMD[1]);
            kill(pid, SIGCONT);
            tcsetpgrp(STDIN_FILENO, pid);
            int status;
            waitpid(pid, &status, WUNTRACED);
            tcsetpgrp(STDIN_FILENO, parentPID);
        }
    } else if (cmd == "bg") {
        if (singleCMD.size() == 2) {
            int pid = stoi(singleCMD[1]);
            kill(pid, SIGCONT);
            processTABLE.insert(pid);
        }
    } else if (cmd == "kill") {
        if (singleCMD.size() == 2) {
            int pid = stoi(singleCMD[1]);
            kill(pid, SIGCONT);
            kill(-pid, SIGINT);
            int status;
            waitpid(pid, &status, WUNTRACED);
            if (processTABLE.find(pid) != processTABLE.end()) {
                processTABLE.erase(pid);
            }
        }
    } else {
        pid_t childPid = fork();
        if (childPid < 0) {
            cout << "CANNOT FORK PROCESS" << endl;
        } else if (childPid == 0) {
            // cout << RED << "Command executed by pid= " << getpid() << DEFAULT << endl;
            // Convert args
            execute(singleCMD);

        } else {
            setpgid(childPid, childPid);
            if (not backgroundCMD) {
                tcsetpgrp(STDIN_FILENO, childPid);
                int status;
                waitpid(childPid, &status, WUNTRACED);
            } else {
                processTABLE.insert(childPid);
            }
            tcsetpgrp(STDIN_FILENO, parentPID);
        }
    }
    return;
}


void executeMultiCMD(vector<string> multiCMD) {
    //cout << "MULTICMDS" << endl;
    int pGID = 0;
    int fd[multiCMD.size()][2];
    for (size_t i = 0; i < multiCMD.size(); ++i) {
        pipe(fd[i]);
        int childPid = fork();
        if (childPid < 0) {
            cout << "CANNOT FORK PROCESS" << endl;
        } else if (childPid == 0) {
            if (i == 0) {
                cout << RED << "Command executed by pid= " << getpid() << DEFAULT << endl;
            }
            if (i != 0) {
                dup2(fd[i-1][0],STDIN_FILENO);
                close(fd[i-1][0]);
                close(fd[i-1][1]);
            }
            if (i != multiCMD.size()-1) {
                dup2(fd[i][1],STDOUT_FILENO);
                close(fd[i][0]);
                close(fd[i][1]);
            }
            execute(parseSingleCMD(multiCMD[i]));
        } else {
            if (i == 0) {
                pGID = childPid;
            } else {
                close(fd[i-1][0]);
                close(fd[i-1][1]);
            }
            setpgid(childPid, pGID);
        }
    }
    if (not backgroundCMD) {
        tcsetpgrp(STDIN_FILENO, pGID);
        for (size_t i = 0; i < multiCMD.size();++i) {
            int status;
            waitpid(-pGID, &status, WUNTRACED);
        }
    } else {
        processTABLE.insert(pGID);
    }
    tcsetpgrp(STDIN_FILENO, parentPID);
}



void userCMD(char* cmd) {

    vector<string> multiCMD = parseMultiCMD(cmd);
    if (multiCMD.size() <= 1) {
        if (multiCMD.empty()) {
            return;
        }
        executeSingleCMD(parseSingleCMD(multiCMD.front()));
    } else {
        executeMultiCMD(multiCMD);
    }
    return;
}


void signalHandler(int sig) {
    //cout << "SIGNAL CAPTURED " << sig << endl;
    if (sig == SIGTSTP) {
        //ctrl-z
        tcsetpgrp(STDIN_FILENO, parentPID);
    } else if (sig == SIGCHLD) {
        int status, pid;
        while ((pid=waitpid(-1, &status, WNOHANG)) > 0) {
            if (processTABLE.find(pid) != processTABLE.end()) {
                processTABLE.erase(pid);
            }
        }
    } else if (sig == SIGINT) {
        if (tcgetpgrp(STDIN_FILENO) == parentPID) {
            fflush(stdin);
            cout << endl;
            userPrompt();
            fflush(stdout);
        }
    }

    return;
}
void init() {
    // SIGNAL Catch init
    signal(SIGCHLD, &signalHandler);
    signal(SIGINT, &signalHandler);
    signal(SIGTSTP, &signalHandler);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    // Global use vars init

    backgroundCMD = false;
    pipeChar = '|';
    spaceChar = ' ';
    getcwd(homePath, sizeof(homePath));
    parentPID = getpid();
    // Login welcome message
    cout << "----Welcome to mysh by 0316222----" << endl;
    //cout << "SHELLPID -- " << parentPID << endl;
    return;
}

int main(int argc, char const *argv[]) {

    // Initialize session
    init();
    char buf[1024];
    while(userPrompt(),cin.getline(buf, sizeof(buf))) {
        backgroundCMD = false;
        userCMD(buf);
    }
    // Detached
    char detach[] = "exit";
    userCMD(detach);

    return 0;
}
