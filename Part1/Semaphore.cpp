#include "Semaphore.hpp"
#include "Headers.hpp"


Semaphore::Semaphore() :size(0) {
    pthread_cond_init(&cond,NULL);
    pthread_mutex_init(&mutex,NULL);
};

Semaphore::Semaphore(unsigned val) :size(val) {
    pthread_cond_init(&cond,NULL);
    pthread_mutex_init(&mutex,NULL);
};

Semaphore::~Semaphore() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
};

void Semaphore:: up(){
    pthread_mutex_lock(&mutex);
    size++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void Semaphore::down() {
    pthread_mutex_lock(&mutex);
    while(size == 0){
        pthread_cond_wait(&cond, &mutex);
    }
    size--;
    pthread_mutex_unlock(&mutex);
}

