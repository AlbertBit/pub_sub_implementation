#include "publisher.h"

Publisher* pub;

void signalHandler(int signo) {

        switch(signo) {

                case SIGINT:
                        delete pub;
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
        int pub2med_write = atoi(argv[3]);

        pub = new Publisher(id, period);
        while(true) {
                if (pub->spinOnce()!=0)
                        return -1;
                if( pub->publish(pub2med_write,  pub->createDefaultMessage()) < 0)
                        return -1;

        }

        return 0;
}
