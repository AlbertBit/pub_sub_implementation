#include "subscriber.h"

int* subscription;
Subscriber* sub;

void signalHandler(int signo) {

        switch(signo) {

                case SIGINT:
                        delete sub;
                        delete[] subscription;
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

        int id = atoi(argv[1]);
        int period = atoi(argv[2]);
        int sub2med_write = atoi(argv[3]);
        int med2sub_read = atoi(argv[4]);
        int nPub = atoi(argv[5]);

        stringstream ss(argv[6]);
        subscription = new int[nPub];
        for(int i = 0; i < nPub; i++) {
                ss >> subscription[i];
        }

        sub = new Subscriber(id, period, subscription, nPub);

        while(true) {

                if (sub->spinOnce()!=0)
                        return -1;

                if( sub->sendRequest(sub2med_write) < 0)
                        return -1;

                string message = sub->getResponse(med2sub_read);
                if(message == "")
                        return -1;

                cout<<"subscriber"<<id<<": "<<message<<endl;
                fflush(stdout);

        }

        return 0;
}
