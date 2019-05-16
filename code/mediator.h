#include <queue>
#include "util.h"

class Mediator {
        private:
                queue<string>* buffer;
                int period;
                int nPub;
                int nSub;
                int maxSize;

                void pop(int id);
                void push(int id, string message);

        public:
                Mediator( int period, int nPub, int nSub, int maxSize );
                ~Mediator();
                int spinOnce();
                //from publisher
                int acceptItem(int* pub2med, fd_set pub2medSet, struct timeval timeout);
                //from and to subscriber
                int acceptRequest(int* sub2med, fd_set sub2medSet, int* med2sub, struct timeval timeout );

                void printAll();

};


Mediator::Mediator( int period, int nPub, int nSub, int maxSize ) {
        buffer = new queue<string>[nPub];
        this->nPub = nPub;
        this->nSub = nSub;
        this->maxSize = maxSize;
        this->period = period;
        cout<<"mediator created"<<endl;

}
Mediator::~Mediator() {
        cout<<"mediator destroyed"<<endl;

        for(int i = 0; i < nPub; i++) {
                while( !buffer[i].empty()) {
                        buffer[i].pop();
                }
        }

        delete[] buffer;
}

void Mediator::push(int id, string message) {
        //applying a fifo policy with limited size
        int n = (int)buffer[id].size();
        if( n == maxSize) {
                buffer[id].pop();
        }
        buffer[id].push(message);
}

void Mediator::pop( int id ) {
        buffer[id].pop();
}

int Mediator::spinOnce( ) {
        int retval = sleep(period);

        if(retval == -1) {
                logMessage("sleep failed");
        }

        return retval;
}


int Mediator::acceptItem( int* pub2med, fd_set pub2medSet, struct timeval timeout ) {


        FD_ZERO(&pub2medSet);

        for(int i = 0; i < nPub; i++) {
                FD_SET(pub2med[i], &pub2medSet);
        }

        if(select(FD_SETSIZE, &pub2medSet, NULL, NULL, &timeout) < 0) {
                logMessage("select failed");
                return -1;
        }

        for(int i = 0; i < nPub; i++) {
                if (FD_ISSET(pub2med[i], &pub2medSet))
                {

                        //first read the length of the message
                        int length = 0;
                        char ch;
                        string message;
                        int nBytes = read(pub2med[i], &length, sizeof(int));
                        if(nBytes < 0 ) {
                                logMessage("read publisher"+to_string(i)+" failed");
                                return -1;
                        }

                        //then read the message itself
                        for(int j = 0; j < length+1; j++) {
                                nBytes = read(pub2med[i], &ch, 1);
                                if(nBytes < 0 ) {
                                        logMessage("read publisher"+to_string(i)+" failed");
                                        return -1;
                                }
                                if(ch != '\0') {
                                        message.push_back(ch);
                                }

                        }
                        //finally push the data in the proper queue
                        push(i, message);
                }
        }
        return 0;
}

int Mediator::acceptRequest( int* sub2med, fd_set sub2medSet, int* med2sub, struct timeval timeout ) {


        FD_ZERO(&sub2medSet);

        for(int i = 0; i < nSub; i++) {
                FD_SET(sub2med[i], &sub2medSet);
        }
        int retval = select(FD_SETSIZE, &sub2medSet, NULL, NULL, &timeout);
        if( retval  < 0) {
                logMessage("select failed");
                return -1;
        }

        for(int i = 0; i < nSub; i++) {
                if (FD_ISSET(sub2med[i], &sub2medSet))
                {
                        int pubId = -1;
                        string message;
                        //read directly the pubId sent by the subscriber
                        int nBytes = read(sub2med[i], &pubId, sizeof(int));
                        if(nBytes < 0 ) {
                                logMessage("read subscriber"+to_string(i)+" failed");
                                return -1;
                        }

                        //go looking for the kind of data to provide
                        //notify to the subscriber with a
                        //special symbol in case no data of that kind is available
                        if(buffer[pubId].empty()) {
                                message = "*";
                        } else {
                                message = buffer[pubId].front();
                                buffer[pubId].pop();
                        }

                        //write length of the data to provide
                        int length = message.length();
                        nBytes = write(med2sub[i], &length, sizeof(int));
                        if( nBytes == -1 ) {
                                logMessage( "write subscriber"+to_string(i)+" failed" );
                                return -1;
                        }
                        //write the data to provide
                        nBytes = write(med2sub[i], message.c_str(), length+1);
                        if( nBytes == -1 ) {
                                logMessage( "transmission failed "+message );
                                return -1;
                        }

                }
        }
        return 0;
}


//debugging utility
void Mediator::printAll() {
        fstream f;
        f.open(MEDFILE, fstream::out);
        for( int i = 0; i < nPub; i++) {
                queue<string> auxQueue(this->buffer[i]);
                int n = (int)auxQueue.size();
                f<<"publisher"<<i<<": ["<<n<<"]  ";

                for( int j = 0; j < n; j++) {
                        f<<auxQueue.front()<<" ";
                        auxQueue.pop();

                }
                f<<endl;
        }
        f.close();

}
