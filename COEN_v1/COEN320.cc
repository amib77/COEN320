
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
pthread_mutex_t qMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t qCond = PTHREAD_COND_INITIALIZER;

// number of threads x 2
#define NUM_THREADS 8

// Vector of queues that hold strings ...we will limit the queues to one entry each
vector< queue<string> > qVector(NUM_THREADS);

// strings parsed from file
vector<string> parsedValues(NUM_THREADS);
//timing constants from project requirements
const int sleeptime[] = {10000,500000,2,100000,5,100000,150000,100000};
int pthreadref[8] = {0,1,2,3,4,5,6,7};
int cthreadref[8] = {0,1,2,3,4,5,6,7};

// consumer and producer functions;
void *consume(void *threadNum);
void *produce(void *threadNum );
void *sconsume(void *threadNum);
void *sproduce(void *threadNum );

bool waitmain = true;


int main(int argc, char* argv[]) {

	ifstream file("data.csv", ifstream::in);
	string line;

	//create producer and consumer threads
	pthread_t pthread[NUM_THREADS];
	pthread_t cthread[NUM_THREADS];

	if (file)
	for(int i = 0; i < NUM_THREADS; i++){
		qVector[i] =  queue< string >();

		switch (i){
			case 0:case 1:case 3:case 5: case 6: case 7:
				pthread_create(&pthread[i], NULL, &produce, &pthreadref[i]);
				pthread_create(&cthread[i], NULL, &consume, &cthreadref[i]);
				break;
			default:
				pthread_create(&pthread[i], NULL, &sproduce, &pthreadref[i]);
				pthread_create(&cthread[i], NULL, &sconsume, &cthreadref[i]);
				break;
		}
	}

	int sleepmain = 1;
	//main will parse the file every second and threads will read parsed data at their respective frequency
	cout<<"Waiting for main data all threads will wait for main to fill queue"<<endl;
	//this is a test to make to see that are threads are waiting on main not needed
	sleep(sleepmain);
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
		cout<<"main--value--update--every--second--"<<endl;
		waitmain = false;
		sleep(sleepmain);

	}

	file.close();
	for(int i = 0; i < NUM_THREADS; i++){
		kill( pthread[i], -1 );
		kill( cthread[i], -1 );

	}
	sleep(1);
	cout<<"THE END"<<endl;
	return 0;
}

void *produce(void *threadNum) {

	const int threadRef =  *((int *) threadNum); //setting the reference integer of thread
	const int threadsleeptime = sleeptime[threadRef];  //set sleep time for current thread
	cout<<threadNum<<" Spawn fast producer: "<< threadRef<<" I run every: "<<threadsleeptime<< "us"<<endl;
	int pcount = 0;     // for console only counts how many times the threads ran
	int produced = 0;    // for console only shows queue size after production;
	string produce;  //this is for console purpose shows the value produced
	while(waitmain);
	while (1)
	{
		//// lock access to data for current running thread
		pthread_mutex_lock(&qMutex);
		////check if data consumed if queue is empty fill it with data from main
		///if it not empty release mutex and let thread wait until condition is met.
		while(!qVector[threadRef].empty())
			pthread_cond_wait(&qCond, &qMutex);
		//////produce value--- here we read value from main and push it in to queue
		qVector[threadRef].push(parsedValues[threadRef]);
		///this is for console visual checking it returns the size of the array
		///at this point size must be one at all time if not one than something is not functioning properly with mutex. deadlock???
		/// this instruction is not needed and needed and affects execution time by the time it takes to complete
		produced = qVector[threadRef].size();
		//the

		pthread_cond_signal( &qCond );
		//// unlock access to data for further use by other threads
		pthread_mutex_unlock( &qMutex );
		///this is for console visual how many times the thread ran
		pcount++;
		cout<<threadRef << ": P "<<qVector[threadRef].front()<<" "<<pcount<<" times run, I produced:"<< produced<<endl;
		///// sleep time
		usleep(threadsleeptime);
	}
}

void *consume(void *threadNum){

	const int threadRef =  *((int *) threadNum);
	const int threadsleeptime = sleeptime[threadRef];

	int ccount=0;
	int consumed = 0;
	string newData;

	cout<<threadNum<<" Spawn fast consumer: "<<threadRef<<" I run every: "<<threadsleeptime<< "us"<<endl;
	while(waitmain);
	while(1) {


		pthread_mutex_lock(&qMutex);
		while(qVector[threadRef].empty())
			pthread_cond_wait(&qCond, &qMutex);

		newData = qVector[threadRef].front();
		consumed = qVector[threadRef].size();
		qVector[threadRef].pop();
	    //Now unlock the mutex
	    pthread_cond_signal( &qCond );
	    pthread_mutex_unlock( &qMutex );

	    ccount++;
	    cout<<threadRef << ": C "<<newData<<" "<< ccount<<" times run, I consumed:"<< consumed<<endl;

	    // - and process the new data
	    usleep(threadsleeptime);	  // opted to put sleep here as to handle all mutex instruction first and then
	}
}

void *sproduce(void *threadNum) {

	const int threadRef =  *((int *) threadNum); //setting the reference integer of thread
	const int threadsleeptime = sleeptime[threadRef];  //set sleep time for current thread
	cout<<threadNum<<" Spawn fast producer: "<< threadRef<<" I run every: "<<threadsleeptime<< "us"<<endl;
	int pcount = 0;     // for console only counts how many times the threads ran
	int produced = 0;    // for console only shows queue size after production;
	string produce;  //this is for console purpose shows the value produced
	while(waitmain);
	while (1)
	{
		//// lock access to data for current running thread
		pthread_mutex_lock(&qMutex);
		////check if data consumed if queue is empty fill it with data from main
		///if it not empty release data and let thread wait until condition is met.
		while(!qVector[threadRef].empty())
			pthread_cond_wait(&qCond, &qMutex);
		//////produce value--- here we read value from main and push it in to queue
		qVector[threadRef].push(parsedValues[threadRef]);
		///this is for console visual checking it returns the size of the array
		///at this point size must be one at all time if not one than something is not functioning properly with mutex. deadlock???
		/// this instruction is not needed and needed and affects execution time by the time it takes to complete
		produced = qVector[threadRef].size();
		//the
		pthread_cond_signal( &qCond );
		//// unlock access to data for further use by other threads
		pthread_mutex_unlock( &qMutex );
		///this is for console visual how many times the thread ran
		pcount++;

		cout<<threadRef << ": P "<<qVector[threadRef].front()<<" "<<pcount<<" times run, I produced:"<< produced<<endl;
		///// sleep time
		sleep(threadsleeptime);
	}
}

void *sconsume(void *threadNum){

	const int threadRef =  *((int *) threadNum);
	int ccount=0;

	const int threadsleeptime = sleeptime[threadRef];
	cout<<threadNum<<"  Spawn slow consumer: "<<threadRef<<" I run every: "<<threadsleeptime<< "s"<<endl;
	int consumed = 0;
	while(waitmain);
	while(1) {

		pthread_mutex_lock(&qMutex);
		while(qVector[threadRef].empty())
			pthread_cond_wait(&qCond, &qMutex);
		string newData = qVector[threadRef].front();
		consumed = qVector[threadRef].size();
		qVector[threadRef].pop();

	    //Now unlock the mutex
	    pthread_cond_signal( &qCond );
	    pthread_mutex_unlock( &qMutex );



	    ccount++;
	    cout<<threadRef << ": C "<<newData<<" "<< ccount<<" times run, I consumed:"<< consumed<<endl;
	    sleep(threadsleeptime);
	}
	    // - and process the new data
}

