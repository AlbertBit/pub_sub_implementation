#include "util.h"

class Subscriber {

        private:

                int id;
                int period;
                long messageCounter;
                vector<int> subscription;

        public:

                Subscriber();
                Subscriber(int id, int period, int* subscription, int nPub);
                ~Subscriber();
                void setId(int period);
                void setPeriod(int period);
                void setSubscription(int* subscription, int nPub);

                int sendRequest(int sub2med);
                string getResponse(int med2sub);
                int spinOnce();

};

Subscriber::Subscriber() {
        messageCounter = 0;
        id = -1;
        period = -1;
        subscription.clear();

}

Subscriber::Subscriber(int id, int period, int* subscription, int nPub) {
        this->id = id;
        this->period = period;
        messageCounter = 0;

        this->subscription.clear();

        //insert publisher id into the subscription list
        for( int i = 0; i < nPub; i++ ) {
                if( subscription[i] == 1 ) {
                        this->subscription.push_back(i);
                }
        }

        cout<<"subscriber created: {id="<<id<<", period="<<period<<"}"<<endl;
}

Subscriber::~Subscriber() {
        subscription.clear();
        cout<<"subscriber"<<id<<" destroyed"<<endl;
}

void Subscriber::setId( int id) {
        this->id = id;
}

void Subscriber::setPeriod( int period) {
        this->period = period;
}

void Subscriber::setSubscription( int* subscription, int nPub) {
        this->subscription.clear();

        for( int i = 0; i < nPub; i++ ) {
                if( subscription[i] == 1 ) {
                        this->subscription.push_back(i);
                }
        }
}

int Subscriber::sendRequest(int sub2med) {

        //ONE request of data, for ONE of the publishers
        //to which there is subscription at each cycle
        int nPub = subscription.size();
        int pubId = subscription[messageCounter%nPub];

        int nBytes = write(sub2med, &pubId, sizeof(int));
        if( nBytes == -1 ) {
                logMessage( "subscribing failed "+to_string(pubId) );
                return -1;
        }
        messageCounter++;
        return pubId;
}

string Subscriber::getResponse(int med2sub){


        char ch;
        string message;
        int length = -1;
        //read length of the response
        int nBytes = read(med2sub, &length, sizeof(int));
        if(nBytes < 0 ) {
                logMessage("read subscriber failed");
                return "";
        }

        //return the response
        for(int i = 0; i < length+1; i++) {
                nBytes = read(med2sub, &ch, 1);
                if(nBytes < 0 ) {
                        logMessage("read subscriber failed");
                        return "";
                }
                if(ch != '\0') {
                        message.push_back(ch);
                }
        }

        return message;
}

//sleeping routine
int Subscriber::spinOnce() {
        int retval = sleep(period);

        if(retval == -1) {
                logMessage( "sleep failed" );
        }

        return retval;
}
