//
// Created by orsht on 5/25/2019.
//
#include "Thread.hpp"
#include "Game.hpp"
#ifndef OS_WET3_THREAD_CONSUMER_H
#define OS_WET3_THREAD_CONSUMER_H

class Thread_Consumer : public Thread{
    Game* game;

public:
    Thread_Consumer(uint m_thread_id, Game* game) : Thread(m_thread_id), game(game) {};

protected:
    void thread_workload() override{
        while(true){
            auto task = game->getTask_queue()->pop();
            if(task.start_raw == -1)
                break;
            auto tile_start = std::chrono::system_clock::now();
            game->game_of_life_calc(task);
            auto tile_end = std::chrono::system_clock::now();
            tile_record rec = {((double)std::chrono::duration_cast<std::chrono::microseconds>(tile_end - tile_start).count()), m_thread_id};

            // add time stamps and shit

            pthread_mutex_lock(&game->active_threads_lock);
            auto hist = game->tile_hist();
            hist.push_back(rec);
            --game->active_threads;
            pthread_cond_signal(&game->active_threads_cond);
            pthread_mutex_unlock(&game->active_threads_lock);
        }
    }
};

#endif //OS_WET3_THREAD_CONSUMER_H
