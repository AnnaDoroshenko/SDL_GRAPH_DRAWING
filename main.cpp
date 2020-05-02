#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <SDL2/SDL.h>
#include <cmath>


//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;


SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL; //The window renderer


// ========== Data types ======== //

struct Transmission {
	public:
		unsigned int begin_at;
		unsigned int finish_at;
		unsigned int proc_dest;

	public:
		Transmission(const unsigned int& begin_at, const unsigned int& finish_at, 
					const unsigned int& proc_dest) : begin_at(begin_at), 
					finish_at(finish_at), proc_dest(proc_dest) {}

		friend std::ostream& operator<<(std::ostream& os, const Transmission& transmission);
};


struct Chunk {
	public:
		unsigned int proc_num;
		std::string name;
		unsigned int begin_at;
		unsigned int finish_at;
		std::vector<Transmission> transmissions;

	public:
		Chunk(const unsigned int& proc_num, const std::string& name,
			const unsigned int& begin_at, const unsigned int& finish_at,
			const std::vector<Transmission>& transmissions) :
			proc_num(proc_num), name(name), begin_at(begin_at),
			finish_at(finish_at), transmissions(transmissions) {}

		friend std::ostream& operator<<(std::ostream& os, const Chunk& chunk);
};


std::ostream& operator<<(std::ostream& os, const Transmission& trans) {
	os << " T(b: " << trans.begin_at << ", f: " << trans.finish_at << ", dest: " << trans.proc_dest << ")";
	return os;
}


std::ostream& operator<<(std::ostream& os, const Chunk& chunk) {
	os << "Chunk(proc: " << chunk.proc_num << ", name: " << chunk.name << ", b: " << chunk.begin_at
		<< ", f: " << chunk.finish_at << ", transmissions:";

	if (!chunk.transmissions.empty()) {
		for (unsigned int i = 0; i < chunk.transmissions.size(); i++)
        {
            os << chunk.transmissions[i] << "";
        }
	} else {
		os << " none";
	}
	os << ")";

	return os;
}
// =========================================================


// ==================== Methods for SDL ==================== //
bool init() {
	
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
		success = false;
	} else {
		// Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
			std::cout << "Warning: Linear texture filtering not enabled!" << std::endl;
		}

		// Create window
		gWindow = SDL_CreateWindow("Little SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL) {
			std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
			success = false;
		} else {
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL) {
				std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
				success = false;
			} else {
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			}
		}
	}


	return success;
}


void close() {
	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}


std::pair<unsigned int, unsigned int> getUnits(const std::vector<Chunk>& chunks);


void drawGraph(const std::vector<Chunk>& chunks) {
	const std::pair<unsigned int, unsigned int> counted = getUnits(chunks);
	std::cout << "(x_unit = " << counted.first << ", y_unit = " << counted.second << ")" << std::endl;
	unsigned int x_unit = counted.first;
	unsigned int y_unit = counted.second;

	//Start up SDL and create window
	if (!init()) {
		std::cout << "Failed to initialize!" << std::endl;
	} else {		
		bool quit = false;
		SDL_Event e;

		while (!quit) {
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
					quit = true;
				}
			}

			//Clear screen
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(gRenderer);

			std::vector<SDL_Rect> rectangulars;

			for (const auto& chunk : chunks) {
				// TODO: add margin
				const auto calculate_begin = [x_unit](auto elem) {
					return elem.begin_at * x_unit;
					// return margin + elem.begin_at * x_unit;
				};

				const auto calculate_finish = [x_unit](auto elem) {
					return elem.finish_at * x_unit;
					// return margin + elem.finish_at * x_unit;
				};

				unsigned int x = calculate_begin(chunk);
				unsigned int elems_before = 0;
				for (unsigned int i = 0; ((i < chunk.proc_num) && (i < trans_count_max.size())); i++) {
					if (trans_count_max != -1) {
						elems_before += trans_count_max[i];
					}
				}
				// unsigned int y = margin + elems_before * y_unit;
				unsigned int y = elems_before * y_unit;
				unsigned int width = calculate_finish(chunk);
				unsigned int height = y_unit;
				const SDL_Rect proc_rect = {x, y, width, height};
				rectangulars.push_back(proc_rect);
				const std::vector<Transmission> curr_transmissions = chunk.transmissions;
				if (!curr_transmissions.empty()) {
					for (unsigned int j = 0; j < curr_transmissions.size(); j++) {
						x = calculate_begin(curr_transmissions[j]);
						y += (j + 1) * y_unit; // +1 as index starts with 0
						width = calculate_finish(curr_transmissions[j]);
						const SDL_Rect trans_rect = {x, y, width, height};
						rectangulars.push_back(trans_rect);
					}
				}
			}

			if (!rectangulars.empty()) {
				SDL_RenderDrawRects(gRenderer, &rectangulars[0], rectangulars.size());
			}

			//Update screen
			SDL_RenderPresent( gRenderer );
		}
	}

	//Free resources and close SDL
	close();
}

// ========================================================= //


// ============= Tools ================ //

std::pair<unsigned int, unsigned int> getUnits(const std::vector<Chunk>& chunks) {
	// calculation of x_unit
	unsigned int max_x = 0;
	unsigned int max_proc_num = 0;
	for (const auto& chunk : chunks) {
		unsigned int curr_max_time = chunk.finish_at;
		unsigned int curr_proc_num = chunk.proc_num;
		if (!chunk.transmissions.empty()) {
			curr_max_time = chunk.transmissions.back().finish_at;
		}
		if (curr_max_time > max_x) max_x = curr_max_time;
		if (curr_proc_num > max_proc_num) max_proc_num = curr_proc_num;
	}

	std::vector<int> trans_count_max(max_proc_num + 1, -1);
	for (const auto& chunk : chunks) {
		const unsigned int curr_proc_num = chunk.proc_num;
		const int curr_trans_size = chunk.transmissions.size();
		if (curr_trans_size > trans_count_max[curr_proc_num])
			trans_count_max[curr_proc_num] = curr_trans_size;
	}

	// calculation of y_unit
	unsigned int sum_y = 0;
	for (const auto& trans_count : trans_count_max) {
		if (trans_count != -1) {
			sum_y += trans_count + 1; // + 1 for the main chunk
		}	
	}


	return {(unsigned int) SCREEN_WIDTH / max_x, (unsigned int) SCREEN_HEIGHT / sum_y};
}
// ================================================== //

int main() {

	std::vector<Chunk> chunks;
	chunks.push_back(Chunk(1, "A", 0, 2, std::vector<Transmission>{}));
	chunks.push_back(Chunk(1, "B", 2, 4, std::vector<Transmission>{}));

	std::vector<Transmission> transmissions1;
	transmissions1.push_back(Transmission(3, 4, 1));
	transmissions1.push_back(Transmission(3, 4, 3));

	chunks.push_back(Chunk(2, "C", 0, 3, transmissions1));

	std::vector<Transmission> transmissions2;
	transmissions2.push_back(Transmission(5, 6, 1));

	chunks.push_back(Chunk(3, "D", 4, 5, transmissions2));
	chunks.push_back(Chunk(1, "E", 6, 7, std::vector<Transmission>{}));

	for (auto& chunk : chunks) {
		std::cout << chunk << std::endl;
	}

	// const std::pair<float, unsigned int> counted = getUnits(chunks);
	// std::cout << "(width = " << counted.first << ", height = " << counted.second << ")" << std::endl;
	drawGraph(chunks);

	
	return 0;
}