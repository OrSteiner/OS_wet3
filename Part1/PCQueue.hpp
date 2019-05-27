#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
#include "Semaphore.hpp"

// Single Producer - Multiple Consumer queue

template <typename T>class PCQueue
{

public:

	PCQueue(): qsize(0), writer(false), reader(false){
		pthread_cond_init(&waiting_writer,NULL);
		pthread_mutex_init(&cond_mutex,NULL);
		pthread_mutex_init(&critical_mutex,NULL);
	};
	// Blocks while queue is empty. When queue holds items, allows for a single
	// thread to enter and remove an item from the front of the queue and return it. 
	// Assumes multiple consumers.

	T pop(){
		/*
		 * Preventing empty queue access.
		 */
		qsize.down();

		/*
		 * We want to make sure, that only up to one reader and one writer will be able to wait on critical mutex.
		 */
		pthread_mutex_lock(&cond_mutex);
		while(writer || reader){
			pthread_cond_wait(&waiting_writer, &cond_mutex);
		}
		reader = true;
		pthread_mutex_unlock(&cond_mutex);
		/*
		 * Critical section.
		 */
		pthread_mutex_lock(&critical_mutex);
		T item = pc_queue.front();
		pc_queue.pop();
		pthread_mutex_unlock(&critical_mutex);
		/*
		 * Enabling the next reader to come in and compete for critical part with the writer (if waiting).
		 */
		pthread_mutex_lock(&cond_mutex);
		reader = false;
		pthread_cond_broadcast(&cond_mutex);
		pthread_mutex_unlock(&cond_mutex);
		return item;
	};

	// Allows for producer to enter with *minimal delay* and push items to back of the queue.
	// Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.  
	// Assumes single producer

	void push(const T& item){
		/*
		 * Anouncing writers arrival.
		 */
		pthread_mutex_lock(&cond_mutex);
		writer = true;
		pthread_mutex_unlock(&cond_mutex);
		/*
		 * Critical section.
		 */
		pthread_mutex_lock(&critical_mutex);
		pc_queue.push(item);
		pthread_mutex_unlock(&critical_mutex);

		qsize.up();
		/*
		 * Releasing the readers from wating queue.
		 */
		pthread_mutex_lock(&cond_mutex);
		writer = false;
		pthread_cond_broadcast(&cond_mutex);
		pthread_mutex_unlock(&cond_mutex);
	};


private:
	queue<T> pc_queue;						// The tasks queue.
	Semaphore qsize;					// Semaphore for preventing access to empty queue.
	pthread_mutex_t critical_mutex;		// Critical section lock - Max of one reader and one writer will wait here.
	pthread_mutex_t cond_mutex;			// The lock for conditions changes.
	pthread_cond_t waiting_writer;		// Cond var for readers wainting queue.
	bool writer, reader;				// Is there a reader/writer in action.

};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif