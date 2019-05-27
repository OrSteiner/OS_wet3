#include "Game.hpp"
#include "Thread_Consumer.hpp"
#include "utils.hpp"
/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
void Game::run() {

	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation 
		auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(nullptr);
	} // generation loop
	print_board("Final Board");
	_destroy_game();
}

void Game::_init_game() {

	// Create game fields - Consider using utils:read_file, utils::split
	// Create & Start threads
	// Testing of your implementation will presume all threads are started here
	for(uint i = 0; i < m_thread_num; ++i){
		auto thread = new Thread_Consumer(i, this);
		m_threadpool.push_back(thread);
		m_threadpool[i]->start();
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
    while(!active_threads){
        pthread_cond_wait(&active_threads_cond, &active_threads_lock);
    }
    active_threads = m_thread_num;
    pthread_mutex_unlock(&active_threads_lock);
    // Swapping the board matrices.
    bool_mat* temp = current_board;
    current_board = next_board;
    next_board = temp;

}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources 
	// Not implemented in the Game's destructor for testing purposes. 
	// All threads must be joined here
    int start = -1;
    int end = -1;
    for(int i = 0; i < m_thread_num; ++i){
        task item;
        item.start_raw = start;
        item.end_raw = end;
        task_queue->push(item);
    }
	for (uint i = 0; i < m_thread_num; ++i) {
        m_threadpool[i]->join();
    }
    for (uint i = 0; i < m_thread_num; ++i) {
        delete(m_threadpool[i]);
    }
}

void Game::game_of_life_calc(task item){
    int width = current_board[0].size();
    int life_counter = 0;
    for(int i = item.start_raw; i <= item.end_raw; ++i){    // Calculate for each pixel
        for(int j = 0; j < width; ++j){
            if(i-1 >= 0){                   // First row calculation
                if(j-1 >= 0){
                    life_counter += (*current_board)[i-1][j-1];
                }
                life_counter += (*current_board)[i-1][j];
                if(j+1 < width){
                    life_counter += (*current_board)[i-1][j+1];
                }
            }
            if(j-1 >= 0){
                life_counter += (*current_board)[i][j-1];
            }
            life_counter += (*current_board)[i][j];
            if(j+1 < width){
                life_counter += (*current_board)[i][j+1];
            }
            if(i+1 < num_of_rows){
                if(j-1 >= 0){
                    life_counter += (*current_board)[i+1][j-1];
                }
                life_counter += (*current_board)[i+1][j];
                if(j+1 < width){
                    life_counter += (*current_board)[i+1][j+1];
                }
            }
            if(life_counter == 3){
                (*next_board)[i][j] = true;
            }
            else if(life_counter == 2 && (*current_board)[i][j]){
                (*next_board)[i][j] = true;
            }
            else{
                (*next_board)[i][j] = false;
            }
        }
    }


}

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {

	if(print_on){ 

		// Clear the screen, to create a running animation 
		if(interactive_on)
			system("clear");

		// Print small header if needed
		if (header != nullptr)
			cout << "<------------" << header << "------------>" << endl;
		
		// TODO: Print the board 

		// Display for GEN_SLEEP_USEC micro-seconds on screen 
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

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
    vector<string> matrix_by_raws = utils::read_lines(filename);
    num_of_rows = (uint) matrix_by_raws.size();
    vector<string> string_single_raw = utils::split(matrix_by_raws[0],' ');
    int num_of_columns = (uint) string_single_raw.size();
    for(int i=0; i<num_of_rows; i++){
        vector<string> single_raw = utils::split(matrix_by_raws[i],' ');
        vector<bool> bool_single_raw;
        for(int j=0; j<num_of_columns; j++){
            bool_single_raw.push_back((bool)std::stoi(string_single_raw[j]));
        }
        current_board->push_back(bool_single_raw);
    }
    m_thread_num = thread_num();
};

Game::~Game() {

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
