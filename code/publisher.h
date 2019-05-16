#include "util.h"

class Publisher {

        private:

                int id;
                int period;
                long messageCounter;

        public:

                Publisher();
                Publisher(int id, int period);
                ~Publisher();
                void setId(int period);
                void setPeriod(int period);
                string createDefaultMessage();
                int publish(int fd, string message);
                int spinOnce();

};

Publisher::Publisher() {
        messageCounter = 0;
        id = -1;
        period = -1;

}

Publisher::Publisher(int id, int period) {
        this->id = id;
        this->period = period;
        messageCounter = 0;
        cout<<"publisher created: {id="<<id<<", period="<<period<<"}"<<endl;
}

Publisher::~Publisher() {
        cout<<"publiher"<<id<<" destroyed"<<endl;
}

void Publisher::setId( int id) {
        this->id = id;
}

void Publisher::setPeriod( int period) {
        this->period = period;
}

string Publisher::createDefaultMessage() {

        //messageCounter used to construct the message to send
        char ch = (char)('a'+(messageCounter%26));
        string message = ch+to_string(id) ;
        return message;
}

//in principle every kind of string
int Publisher::publish(int fd, string message){

        //first write length of the message
        int length = message.length();
        int nBytes = write(fd, &length, sizeof(int));
        if( nBytes == -1 ) {
                logMessage( "publishing failed "+message );
                return -1;
        }
        //then write the message itself
        nBytes = write(fd, message.c_str(), length+1);
        if( nBytes == -1 ) {
                logMessage( "publishing failed "+message );
                return -1;
        }
        messageCounter++;
        return nBytes;
}


int Publisher::spinOnce() {
        int retval = sleep(period);
        if(retval == -1) {
                logMessage( "sleep failed" );
        }

        return retval;
}
