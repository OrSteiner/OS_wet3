#include "PCQueue.hpp"

template <class T> // Add definition in PCQueue.hpp if you need this constructor
PCQueue<T>::PCQueue():
        readCount(0),
        writeCount(0),
        readCountSem(1),
        writeCountSem(1),
        readAccessSem(1),
        criticalSem(1),
        q()
/* Init list*/ {
}

//std::pair<T, std::chrono::time_point<std::chrono::system_clock>> PCQueue<T>::pop(){
template <class T>
T PCQueue<T>:: pop(){
    qSize.down();
    readAccessSem.down();
    readCountSem.down();
    readCount++;

    if(readCount==1){
        criticalSem.down();
    }
    readCountSem.up();
    readAccessSem.up();

   ////////////critical part
    T item=q.front();
    q.pop();
   ///////////end of critical part
    readCountSem.down();
    readCount--;
    if(readCount==0){
        criticalSem.up();
    }
    readCountSem.up();
    std::pair<T, std::chrono::time_point<std::chrono::system_clock>> temp(item,std::chrono::system_clock::now());
    return temp;
}

template <class T>
void PCQueue<T>::push(const T& item){
    writeCountSem.down();
    writeCount++;
    if(writeCount == 1){ //should always enter
        readAccessSem.down();
    }
    writeCountSem.up();
    criticalSem.down();
    q.push(item);
    criticalSem.up();
    writeCountSem.down();
    writeCount--;
    if(writeCount == 0){ //should always enter
        readAccessSem.up();
    }
    writeCountSem.up();
    qSize.up();
}