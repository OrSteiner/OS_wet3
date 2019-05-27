#include "Game.hpp"
#include "utils.hpp"


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


/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
void Game::run() {

	_init_game(); // Starts the threads and all other variables you need
	print_board_clion("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation 
		auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board_clion(nullptr);
	} // generation loop
	print_board_clion("Final Board");
	_destroy_game();
}

void Game::_init_game() {

	// Create game fields - Consider using utils:read_file, utils::split
	// Create & Start threads
	// Testing of your implementation will presume all threads are started here
	for(uint i = 0; i < m_thread_num; ++i){
		auto thread = new Thread_Consumer(i, this);
		m_threadpool.push_back(thread);
		m_threadpool.at(i)->start();
	}
}

void Game::_step(uint curr_gen) {
	// Push jobs to queue
	// Wait for the workers to finish calculating 
	// Swap pointers between current and next field 
	// NOTE: Threads must not be started here - doing so will lead to a heavy penalty in your grade
    int start = 0;
    int num_of_rows_per_task = num_of_rows/m_thread_num;
    int end = num_of_rows_per_task - 1;
    for(int i = 0; i < m_thread_num; ++i){
        task item;
        item.start_raw = start;
        if(i == m_thread_num-1){
            item.end_raw = num_of_rows - 1;
        }
        else {
            item.end_raw = end;
        }
        start += num_of_rows_per_task;
        end += num_of_rows_per_task;
        task_queue->push(item);
    }
    // Waiting on all threads finishing this generation's tasks.
    pthread_mutex_lock(&active_threads_lock);
    while(active_threads){
        pthread_cond_wait(&active_threads_cond, &active_threads_lock);
    }
    active_threads = m_thread_num;
    pthread_mutex_unlock(&active_threads_lock);
    // Swapping the board matrices.
    //bool_mat* temp = current_board;
    current_board = next_board;
    //next_board = temp;

}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources 
	// Not implemented in the Game's destructor for testing purposes. 
	// All threads must be joined here
    int start = -1;
    int end = -1;

    // Inserting poison tasks into task queue.

    for(int i = 0; i < m_thread_num; ++i){
        task item;
        item.start_raw = start;
        item.end_raw = end;
        task_queue->push(item);
    }

    // Waiting with main thread for all created threads to exit.

	for (uint i = 0; i < m_thread_num; ++i) {
        m_threadpool.at(i)->join();
    }

    // Freeing the memory allocated by new on Threads' creation.

    for (uint i = 0; i < m_thread_num; ++i) {
        delete(m_threadpool.at(i));
    }
}

void Game::game_of_life_calc(task item){
    int width = (int)current_board.at(0).size();
    int life_counter = 0;
    for(int i = item.start_raw; i <= item.end_raw; ++i){    // Calculate for each pixel
        for(int j = 0; j < width; ++j){
            if(i-1 >= 0){                   // First row calculation
                if(j-1 >= 0){
                    life_counter += (current_board).at(i-1).at(j-1);
                }
                life_counter += (current_board).at(i-1).at(j);
                if(j+1 < width){
                    life_counter += (current_board).at(i-1).at(j+1);
                }
            }
            if(j-1 >= 0){
                life_counter += (current_board).at(i).at(j-1);
            }
            if(j+1 < width){
                life_counter += (current_board).at(i).at(j+1);
            }
            if(i+1 < num_of_rows){
                if(j-1 >= 0){
                    life_counter += (current_board).at(i+1).at(j-1);
                }
                life_counter += (current_board).at(i+1).at(j);
                if(j+1 < width){
                    life_counter += (current_board).at(i+1).at(j+1);
                }
            }
            if(life_counter == 3){
                (next_board).at(i).at(j) = true;
            }
            else if(life_counter == 2 && (current_board).at(i).at(j)){
                (next_board).at(i).at(j) = true;
            }
            else{
                (next_board).at(i).at(j) = false;
            }
        life_counter = 0;
        }
    }


}

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {

    int width = (int)(current_board.at(0).size());
    int hight = (int)current_board.size();

	if(print_on){ 

		// Clear the screen, to create a running animation 
		if(interactive_on)
			system("clear");

		// Print small header if needed
		if (header != nullptr)
			cout << "<------------" << header << "------------>" << endl;

        cout << u8"╔" << string(u8"═") * width << u8"╗" << endl;
        for (uint i = 0; i < hight; ++i) {
            cout << u8"║";
            for (uint j = 0; j < width; ++j) {
                cout << (current_board.at(i).at(j) ? u8"█" : u8"░");
            }
            cout << u8"║" << endl;
        }
        cout << u8"╚" << string(u8"═") * width << u8"╝" << endl;

		// Display for GEN_SLEEP_USEC micro-seconds on screen 
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

}

inline void Game::print_board_clion(const char* header){
    int width = (int)(current_board.at(0).size());
    int hight = (int)current_board.size();

    //cout << "_" << string(u8"═") * width << "_" << endl;
    for (uint i = 0; i < hight; ++i) {
        //cout << u8"║";
        for (uint j = 0; j < width; ++j) {
            cout << (current_board.at(i).at(j) ? 1 : 0);
        }
        cout << endl;
    }
    //cout << u8"╚" << string(u8"═") * width << u8"╝" << endl;
            cout << endl << endl;
}

PCQueue<task> *Game::getTask_queue() const {
	return task_queue;
}


/* Function sketch to use for printing the board. You will need to decide its placement and how exactly 
	to bring in the field's parameters. 

		cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
		for (uint i = 0; i < field_height ++i) {
			cout << u8"║";
			for (uint j = 0; j < field_width; ++j) {
				cout << (field[i][j] ? u8"█" : u8"░");
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
*/

/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/
Game::Game(game_params params) : m_gen_num(params.n_gen), m_thread_num(params.n_thread),
                                 interactive_on(params.interactive_on), print_on(params.print_on),
                                 filename(params.filename) {
    vector<string> matrix_by_rows = utils::read_lines(filename);
    num_of_rows = (uint) matrix_by_rows.size();
    vector<string> string_single_raw = utils::split(matrix_by_rows.at(0),' ');
    int num_of_columns = (uint) string_single_raw.size();
    for(int i=0; i<num_of_rows; i++){
        vector<string> single_row = utils::split(matrix_by_rows.at(i),' ');
        vector<bool> bool_single_row;
        for(int j=0; j<num_of_columns; j++){
            bool_single_row.push_back((bool)std::stoi(single_row.at(j)));
        }
        current_board.push_back(bool_single_row);
    }
    m_thread_num = thread_num();
    task_queue = new PCQueue<task>();
    active_threads = m_thread_num;
    pthread_cond_init(&active_threads_cond, NULL);
    pthread_mutex_init(&active_threads_lock, NULL);
    next_board.resize(num_of_rows);
    for(int i = 0; i < num_of_rows; ++i){
        next_board.at(i).resize(num_of_columns);
    }
};

Game::~Game(){
    delete(task_queue);
}

uint Game::thread_num() const {
	return __min(m_thread_num, num_of_rows);
}

const vector<double> Game::gen_hist() const{
    return m_gen_hist;
}
const vector<tile_record> Game::tile_hist() const{
    return m_tile_hist;
}

/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/