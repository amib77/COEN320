#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <vector>
#include <pthread.h>
#include <unistd.h>

using namespace std;
//mutex and condition for protecting and managing queues
pthread_mutex_t qMutex;
pthread_cond_t qCond;

// number of threads
#define NUM_THREADS 8



// Vector of queues that hold strings ...we will limit the queues to one entry each
vector< queue<string> > qVector(NUM_THREADS);

// strings parsed from file
vector<string> parsedValues(NUM_THREADS);
//timing constants from project requirements
const double sleeptime[] = {0.01,0.5,2,1,5,0.1,0.15,0.2};

// consumer and producer functions;
void *consume(void *threadNum);
void *produce(void *threadNum );


int main(int argc, char* argv[]) {

	ifstream file("data.csv", ifstream::in);
	string line;

	//create producer and consumer threads
	pthread_t pthread[NUM_THREADS];
	pthread_t cthread[NUM_THREADS];

	if (file)
	for(int i = 0; i < NUM_THREADS; i++){
		int j = i;
		qVector[i] = queue<string>();
		int *arg = new int;
		*arg = j;
		//initiate threads
		pthread_create(&pthread[i], NULL, &produce, arg);
		pthread_create(&cthread[i], NULL, &consume, arg);

	}
	sleep(2);

	int count =0;
	//main will parse the file every second and threads will read parsed data at their respective frequency
	if (file)
	while (getline(file, line))
	{
		int pos=0;
		for (int i = 0; i < 8; i++) {
			pos = line.find(",");
			pthread_mutex_lock(&qMutex);
			parsedValues[i] = line.substr(0, pos);
			pthread_mutex_unlock(&qMutex);
			line = line.substr(pos + 1);
		}

		sleep(1);
		cout<<"main"<<endl;

		if(count == 0){
			break;
		}
		count++;

	}

	file.close();
	for(int i = 0; i < NUM_THREADS; i++){
		kill( pthread[i], -1 );
		kill( cthread[i], -1 );

	}
	return 0;
}

void *produce(void *threadNum) {

	const int threadRef =  *((int *) threadNum);
	cout<<"spt: "<< threadRef<<endl;



	while (1)
	{
		pthread_mutex_lock(&qMutex);
		while(!qVector[threadRef].empty()) {
			pthread_cond_wait(&qCond, &qMutex);
		}

		qVector[threadRef].push(parsedValues[threadRef]);
		cout<< threadRef << " p "<< qVector[threadRef].front() << endl;
		pthread_cond_signal(&qCond);
		pthread_mutex_unlock(&qMutex);
		sleep(sleeptime[threadRef]);
	}
}


void *consume(void *threadNum){

	const int threadRef =  *((int *) threadNum);

	cout<<"sct: "<< threadRef<<endl;
	sleep(1);
	while(1) {
		pthread_mutex_lock(&qMutex);
		while(qVector[threadRef].empty()) {
			pthread_cond_wait(&qCond, &qMutex);
	    }
	    string newData = qVector[threadRef].front();
	    qVector[threadRef].pop();
	    cout<<threadRef << " c "<< newData << endl;
	    //Now unlock the mutex
	    pthread_mutex_unlock(&qMutex);
	    sleep(sleeptime[threadRef]);
	    // - and process the new data
	}
}


