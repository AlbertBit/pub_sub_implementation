#include "mediator.h"

int* pub2med_read;
int* sub2med_read;
int* med2sub_write;
Mediator* med;

void signalHandler(int signo) {

        switch(signo) {

                case SIGINT:

                        delete med;
                        delete[] pub2med_read;
                        delete[] sub2med_read;
                        delete[] med2sub_write;

                        _exit(0);
                break;

        }
}

int main( int argc, char** argv) {
        ////////////mask setup///////////////
        struct  sigaction sact;//to manage signal mask and disposition
        if( sigemptyset (&sact.sa_mask) == -1) {
                perror("sigemptyset failed");
                exit(-1);
        }
        sact.sa_flags = 0;
        sact.sa_handler = signalHandler;
        //registration of signals
        if(sigaction(SIGINT, &sact, NULL)==-1) {
                perror("sigaction failed");
                exit(-1);
        }

        int nPub = atoi(argv[1]);
        int nSub = atoi(argv[2]);
        int period = atoi(argv[3]);

        //get file filedescriptors
        stringstream p2m_read(argv[4]);
        pub2med_read = new int[nPub];
        for(int i = 0; i < nPub; i++) {
                p2m_read>>pub2med_read[i];
        }

        stringstream s2m_read(argv[5]);
        stringstream m2s_write(argv[6]);
        sub2med_read = new int[nSub];
        med2sub_write = new int[nSub];
        for(int i = 0; i < nSub; i++) {
                s2m_read>>sub2med_read[i];
                m2s_write>>med2sub_write[i];
        }

        int bufferSize = atoi(argv[7]);
        med = new Mediator(period, nPub, nSub, bufferSize);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        fd_set pub2med_readSet;
        fd_set sub2med_readSet;

        while(true) {

                if( med->spinOnce() != 0)
                        return -1;
                if( med->acceptItem(pub2med_read, pub2med_readSet, timeout) != 0)
                        return -1;
                if( med->acceptRequest(sub2med_read, sub2med_readSet, med2sub_write, timeout ) != 0)
                        return -1;

                //print queues on med.txt
                med->printAll();

        }

        return 0;
}
