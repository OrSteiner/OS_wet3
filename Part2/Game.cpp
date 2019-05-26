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
	for(uint int i = 0; i < m_thread_num; ++i){
		Thread_Consumer* thread = new Thread_Consumer(i, this);
		m_threadpool.push_back(thread);
		m_threadpool[i]->start();
	}
}

void Game::_step(uint curr_gen) {
	// Push jobs to queue
	// Wait for the workers to finish calculating 
	// Swap pointers between current and next field 
	// NOTE: Threads must not be started here - doing so will lead to a heavy penalty in your grade 
}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources 
	// Not implemented in the Game's destructor for testing purposes. 
	// All threads must be joined here
	for (uint i = 0; i < m_thread_num; ++i) {
        m_threadpool[j]->join();
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
	int num_of_collumns = (uint) string_single_raw.size();
	for(int i=0; i<num_of_rows; i++){
		vector<string> single_raw = utils::split(matrix_by_raws[i],' ');
		vector<bool> bool_single_raw;
		for(int j=0; j<num_of_collumns; j++){
			bool_single_raw.push_back((bool)std::stoi(string_single_raw[j]));
		}
		current_board->push_back(bool_single_raw);
	}
	m_thread_num = thread_num();
};

uint Game::thread_num() const {
	return __min(m_thread_num, num_of_rows);
}
