#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
using namespace std;

#define READING_SIDE 0
#define WRITING_SIDE 1
#define LOGFILE "./log.txt"
#define MEDFILE "./med.txt"

void logMessage(string message) {

        time_t now = time(0);

        // convert now to string form
        char* dt = ctime(&now);
        string tmp(dt);
        string timestamp = tmp.substr(0, tmp.size()-1);
        fstream f;
        f.open(LOGFILE, fstream::app);
        f<<timestamp<<": "<<message<<endl;
        f.close();
        perror(message.c_str());
}

void initLogFile() {
        fstream f;
        f.open(LOGFILE, fstream::out);
        f.close();
}

bool isParent( pid_t* child,  int n ) {

        bool isParent = true;
        for( int i = 0; i < n; i++ ) {
                if(child[i] <= 0) {
                        isParent = false;
                        break;
                }
        }
        return isParent;
}

void initPids( pid_t* publisherPid, int n ) {

        for( int i = 0; i < n; i++ ) {
                publisherPid[i] = -1;
        }

}

int initPipes( int pipes[][2], int n ) {

        for( int i = 0; i < n; i++ ) {
                if( pipe(pipes[i]) == -1) {
                        perror("pipe failed");
                        string message = "pipe" + to_string(i) + " failed";
                        logMessage(message);
                        return -1;
                }
        }
        return 0;

}
//close all pipes a part from leave open
int closePipes( int pipes[][2], int n, int leaveOpen, int closingSide ) {

        for( int i = 0; i < n; i++ ) {
                if( i != leaveOpen) {
                        if( close(pipes[i][closingSide]) == -1) {
                                perror("close failed");
                                string message = "pipe closing" + to_string(i) + " failed";
                                logMessage(message);
                                return -1;
                        }
                }
        }
        return 0;

}

int forkSet(pid_t* pids, int n) {

        for(int i = 0; i < n; i++) {
                pids[i] = fork();
                if(pids[i] == 0) {
                        break;
                } else if(pids[i] == -1){
                        perror("fork failed");
                        string message = "fork" + to_string(i) + "failed";
                        logMessage(message);
                        return -1;
                }
        }

        return 0;
}

int killSet(pid_t* pids, int n) {

        for(int i = 0; i < n; i++) {
                if(kill(pids[i], SIGINT) == -1) {
                        logMessage("kill failed");
                        return -1;
                }
        }
        return 0;
}

int waitAll( pid_t* publisherPid, pid_t* subscriberPid, pid_t mediatorPid, int n, int m) {

        int wstatus = -1;
        int wpid = -1;

        wpid = waitpid(mediatorPid, &wstatus, 0);
        if( wpid  == -1) {
                perror("waitpid mediator failed");
                return -1;
        }  else {
                cout<<"returned mediator "<<" { pid="<<wpid<<" , status="<<wstatus<<"}"<<endl;
        }

        for(int i = 0; i < n; i++) {
                wpid = waitpid(publisherPid[i], &wstatus, 0);
                if( wpid  == -1 ) {
                        perror("waitpid publisher failed");
                        stringstream ss("");
                        ss <<"returned publisher"<<i<<
                                " { pid="<<wpid<<
                                " , status="<<wstatus<<"}"<<endl;
                        logMessage(ss.str());
                        return -1;
                } else {
                        cout<<"returned publisher"<<i<<
                                " { pid="<<wpid<<
                                " , status="<<wstatus<<"}"<<endl;
                }
        }

        for(int i = 0; i < m; i++) {
                wpid = waitpid(subscriberPid[i], &wstatus, 0);
                if( wpid  == -1) {
                        perror("waitpid subscriber failed");
                        return -1;
                } else {
                        cout<<"returned subscriber"<<i<<
                                " { pid="<<wpid<<
                                " , status="<<wstatus<<"}"<<endl;
                }
        }



        return 0;
}
