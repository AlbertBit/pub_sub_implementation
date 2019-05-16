#include "util.h"

#define N 2 //pubs
#define M 3 //subs
#define NPARAM 4
#define BUFFER_DEFAULT_SIZE 26

pid_t publisherPid[N];
pid_t subscriberPid[M];
pid_t mediatorPid;
void signalHandler(int signo) {

        switch(signo) {

                case SIGINT:
                        if(killSet(publisherPid, N) == -1) {
                                _exit(-1);
                        }
                        if(killSet(subscriberPid, M) == -1) {
                                _exit(-1);
                        }
                        if(killSet(&mediatorPid, 1) == -1) {
                                _exit(-1);
                        }
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
                perror("sigaction USR1 failed");
                exit(-1);
        }

        if(argc != NPARAM) {
                logMessage("wrong number of parameters");
                return 1;
        }

        int pub2med[N][2];
        int sub2med[M][2];
        int med2sub[M][2];

        char* pubFilename = argv[1];
        char* medFilename = argv[2];
        char* subFilename = argv[3];

        int publisherPeriod[N] = {2, 3};
        int subscriberPeriod[M] = {4, 5, 6};
        int mediatorPeriod = 1;

        //1 sub_j gets data from pub_i
        //0 otherwise
        int subscription[M][N]  = {
                                {1, 0},
                                {1, 1},
                                {0, 1},
                                };

        initLogFile();
        initPids(publisherPid, N);
        initPids(subscriberPid, M);

        if( initPipes(pub2med, N) < 0 ) {
                return -1;
        }
        if( initPipes(sub2med, M) < 0 ) {
                return -1;
        }
        if( initPipes(med2sub, M) < 0 ) {
                return -1;
        }

        mediatorPid = fork();
        if( mediatorPid < 0 ) {
                logMessage("mediator fork failed");
                return -1;
        }

        if( mediatorPid == 0) {
                //close unuseful pipes
                closePipes(pub2med, N, -1, WRITING_SIDE);
                closePipes(sub2med, M, -1, WRITING_SIDE);
                closePipes(med2sub, M, -1, READING_SIDE);

                //nPub, nSub, period, queueSize, filedescriptor required
                const char* medArgv[9];

                medArgv[0] = medFilename;

                string nPub = to_string(N);
                medArgv[1] = nPub.c_str();

                string nSub = to_string(M);
                medArgv[2] = nSub.c_str();

                string period = to_string(mediatorPeriod);
                medArgv[3] = period.c_str();

                //publisher to mediator in read mode to receive datas from publishers
                stringstream p2m_read("");
                for (int i = 0; i < N; i++) {
                        p2m_read<<pub2med[i][READING_SIDE];
                        if(i != N-1) {
                                p2m_read<<" ";
                        }
                }
                string p2m_read_str = p2m_read.str();
                medArgv[4] = p2m_read_str.c_str();

                //subscriber to mediator in read mode to receive requests from subscribers
                stringstream s2m_read("");
                //mediator to subscriber in write mode to send response to subscribers
                stringstream m2s_write("");
                for (int i = 0; i < M; i++) {
                        s2m_read<<sub2med[i][READING_SIDE]<<" ";
                        m2s_write<<med2sub[i][WRITING_SIDE]<<" ";
                }

                string s2m_read_str = s2m_read.str();
                medArgv[5] = s2m_read_str.c_str();

                string m2s_write_str = m2s_write.str();
                medArgv[6] = m2s_write_str.c_str();

                //size computed taking the slowest subscriber and the fastest publisher as reference
                int queueSize = *max_element(subscriberPeriod, subscriberPeriod+M) /
                                *min_element(publisherPeriod, publisherPeriod+N);

                if(queueSize < 1 || queueSize > BUFFER_DEFAULT_SIZE) {
                        queueSize = BUFFER_DEFAULT_SIZE;
                }

                string size_str = to_string(queueSize);
                medArgv[7] = size_str.c_str();

                medArgv[8] = NULL;

                if( execv(medFilename, (char**)medArgv) == -1) {
                        logMessage("execv failed");
                }

        } else {
                if( forkSet(publisherPid, N) < 0 ) {
                        return -1;
                }
                if(isParent(publisherPid, N)) {
                        if( forkSet(subscriberPid, M) < 0 ) {
                                return -1;
                        }
                }
                if( isParent(publisherPid, N) && isParent(subscriberPid, M)) {

                        //the init process has now only to wait
                        if( closePipes(pub2med, N, -1, WRITING_SIDE) == -1) {
                                return -1;
                        }
                        if( closePipes(pub2med, N, -1, READING_SIDE) == -1) {
                                return -1;
                        }
                        if( closePipes(sub2med, M, -1, WRITING_SIDE) == -1) {
                                return -1;
                        }
                        if( closePipes(sub2med, M, -1, READING_SIDE) == -1) {
                                return -1;
                        }
                        if( closePipes(med2sub, M, -1, WRITING_SIDE) == -1) {
                                return -1;
                        }
                        if( closePipes(med2sub, M, -1, READING_SIDE) == -1) {
                                return -1;
                        }

                        if( waitAll(publisherPid, subscriberPid, mediatorPid, N, M) == -1) {
                                return -1;
                        }

                } else {

                        for(int i = 0; i < N; i++) {

                                if( publisherPid[i] == 0) {
                                        //close unuseful pipes
                                        if( closePipes(pub2med, N, -1, READING_SIDE) == -1 )
                                                return -1;
                                        if( closePipes(pub2med, N, i, WRITING_SIDE) == -1 )
                                                return -1;
                                        if( closePipes(sub2med, M, -1, READING_SIDE) == -1 )
                                                return -1;
                                        if( closePipes(sub2med, M, -1, WRITING_SIDE) == -1 )
                                                return -1;
                                        if( closePipes(med2sub, M, -1, READING_SIDE) == -1 )
                                                return -1;
                                        if( closePipes(med2sub, M, -1, WRITING_SIDE) == -1 )
                                                return -1;

                                        //id, period, filedes required
                                        const char* pubArgv[5];

                                        pubArgv[0] = pubFilename;

                                        string id = to_string(i);
                                        pubArgv[1] = id.c_str();

                                        string period = to_string(publisherPeriod[i]);
                                        pubArgv[2] = period.c_str();

                                        string p2m_write = to_string(pub2med[i][WRITING_SIDE]);
                                        pubArgv[3] = p2m_write.c_str();

                                        pubArgv[4] = NULL;

                                        if( execv(pubFilename, (char**)pubArgv) == -1) {
                                                logMessage("execv failed");
                                        }
                                }
                        }

                        for(int i = 0; i < M; i++) {

                                if( subscriberPid[i] == 0) {

                                        //close unuseful pipes
                                        if( closePipes(pub2med, N, -1, READING_SIDE) == -1 )
                                                return -1;
                                        if( closePipes(pub2med, N, -1, WRITING_SIDE) == -1 )
                                                return -1;
                                        if( closePipes(sub2med, M, -1, READING_SIDE) == -1 )
                                                return -1;
                                        if( closePipes(sub2med, M, i, WRITING_SIDE) == -1 )
                                                return -1;
                                        if( closePipes(med2sub, M, i, READING_SIDE) == -1 )
                                                return -1;
                                        if( closePipes(med2sub, M, -1, WRITING_SIDE) == -1 )
                                                return -1;

                                        //id, period, nPub, fdes and subscriptions required
                                        const char* subArgv[8];

                                        subArgv[0] = subFilename;

                                        string id = to_string(i);
                                        subArgv[1] = id.c_str();

                                        string period = to_string(subscriberPeriod[i]);
                                        subArgv[2] = period.c_str();

                                        string s2m_write = to_string(sub2med[i][WRITING_SIDE]);
                                        subArgv[3] = s2m_write.c_str();

                                        string m2s_read = to_string(med2sub[i][READING_SIDE]);
                                        subArgv[4] = m2s_read.c_str();

                                        string nPub = to_string(N);
                                        subArgv[5] = nPub.c_str();

                                        stringstream subscr("");
                                        for (int j = 0; j < N; j++) {
                                                subscr<<subscription[i][j]<<" ";
                                        }

                                        string subscr_str = subscr.str();
                                        subArgv[6] = subscr_str.c_str();


                                        subArgv[7] = NULL;

                                        if( execv(subFilename, (char**)subArgv) == -1) {
                                                logMessage("execv failed");
                                        }
                                }
                        }
                }
        }
        return 0;
}
