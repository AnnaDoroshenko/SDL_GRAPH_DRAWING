#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;


SDL_Window*   gWindow   = nullptr; // The window we'll be rendering to
SDL_Renderer* gRenderer = nullptr; // The window renderer


// ========== Data types ======== //

struct Transmission {
    unsigned int begin_at;
    unsigned int finish_at;
    unsigned int proc_dest;

    Transmission(unsigned int begin_at, unsigned int finish_at, unsigned int proc_dest)
        : begin_at(begin_at), finish_at(finish_at), proc_dest(proc_dest) {}

    friend std::ostream& operator<<(std::ostream& os, const Transmission& transmission);
};


struct Subtask {
    unsigned int proc_num;
    std::string name;
    unsigned int begin_at;
    unsigned int finish_at;
    std::vector<Transmission> transmissions;

    Subtask(unsigned int proc_num, const std::string& name,
            unsigned int begin_at, unsigned int finish_at,
            const std::vector<Transmission>& transmissions) :
        proc_num(proc_num), name(name), begin_at(begin_at),
        finish_at(finish_at), transmissions(transmissions) {}

    friend std::ostream& operator<<(std::ostream& os, const Subtask& subtask);
};


struct DrawingBasics {
    std::pair<unsigned int, unsigned int> units;
    std::vector<int> trans_count;

    DrawingBasics(const std::pair<unsigned int, unsigned int>& units,
            const std::vector<int>& trans_count) :
        units(units), trans_count(trans_count) {}
};


struct DrawingElement {
	SDL_Rect rectangle;
	SDL_Surface*  surface;
	SDL_Texture* texture;
	SDL_Color color;

	DrawingElement(const SDL_Rect& rectangle, SDL_Surface*  surface,
			SDL_Texture* texture, const SDL_Color& color) :
		rectangle(rectangle), surface(surface), texture(texture), color(color) {}
};


std::ostream& operator<<(std::ostream& os, const Transmission& trans) {
    os << " T(b: " << trans.begin_at << ", f: " << trans.finish_at << ", dest: " << trans.proc_dest << ")";
    return os;
}


std::ostream& operator<<(std::ostream& os, const Subtask& subtask) {
    os << "Subtask(proc: " << subtask.proc_num << ", name: " << subtask.name << ", b: " << subtask.begin_at
        << ", f: " << subtask.finish_at << ", transmissions:";

    if (subtask.transmissions.empty()) {
        os << " none";
    } else {
        for (const auto& transmission : subtask.transmissions) {
            os << transmission << "";
        }
    }
    os << ")";

    return os;
}
// =========================================================


// ==================== Methods for SDL ==================== //

bool init() {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
    	std::cout << "SDL_ttf could not initialize! SDL Error: " << TTF_GetError() << std::endl;
    	return false;
    }

    // Set texture filtering to linear
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
        std::cout << "Warning: Linear texture filtering not enabled!" << std::endl;
    }

    // Create window
    gWindow = SDL_CreateWindow("Little SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer for window
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == nullptr) {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    //Initialize renderer color
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);


    return true;
}


void close() {
    // Destroy window
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    gRenderer = nullptr;

    // Quit SDL subsystems
    SDL_Quit();
}


DrawingBasics getDrawingBasics(const std::vector<Subtask>& subtasks) {
    // calculation of x_unit
    unsigned int max_x = 0;
    unsigned int max_proc_num = 0;
    for (const auto& subtask : subtasks) {
        unsigned int curr_max_time = subtask.finish_at;
        unsigned int curr_proc_num = subtask.proc_num;
        if (!subtask.transmissions.empty()) {
            curr_max_time = subtask.transmissions.back().finish_at;
        }
        if (curr_max_time > max_x) max_x = curr_max_time;
        if (curr_proc_num > max_proc_num) max_proc_num = curr_proc_num;
    }

    std::vector<int> trans_count_max(max_proc_num + 1, -1);
    for (const auto& subtask : subtasks) {
        const unsigned int curr_proc_num = subtask.proc_num;
        const int curr_trans_size = subtask.transmissions.size();
        if (curr_trans_size > trans_count_max[curr_proc_num])
            trans_count_max[curr_proc_num] = curr_trans_size;
    }

    // calculation of y_unit
    unsigned int sum_y = 0;
    for (const auto& trans_count : trans_count_max) {
        if (trans_count != -1) {
            sum_y += trans_count + 2; // + 2 = 1 * 2 for the Subtask itself (weight == 2)
        }
    }


    return DrawingBasics({SCREEN_WIDTH / max_x, SCREEN_HEIGHT / sum_y},
            trans_count_max);
}


void drawGraph(const std::vector<Subtask>& subtasks) {
    const auto& [units, trans_count] = getDrawingBasics(subtasks);
    const auto& [x_unit, y_unit] = units;
    std::cout << "(x_unit = " << x_unit << ", y_unit = " << y_unit << ")" << std::endl;

    if (!init()) {
        std::cout << "Failed to initialize!" << std::endl;
        return;
    }

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeMono.ttf", 200);
    if (font == nullptr) {
    	std::cout << "Unable to open font" << std::endl;
    	return;
    }

    // Prepare stuff to draw
    const SDL_Color subtask_color = {255, 0, 0, 0};
    const SDL_Color transmission_color = {0, 255, 0, 0};
    std::vector<DrawingElement> drawing_elements;
    std::vector<unsigned int> core_separators;

    for (const auto& subtask : subtasks) {
        // TODO: add margin
        const auto calculate_begin = [x_unit](const auto& elem) {
            return elem.begin_at * x_unit;
            // return margin + elem.begin_at * x_unit;
        };

        const auto calculate_width = [x_unit](const auto& elem) {
            return (elem.finish_at - elem.begin_at) * x_unit;
            // return margin + elem.finish_at * x_unit;
        };

        // unsigned int y = margin + elems_before * y_unit;
        const unsigned int elems_before = [&](){
            unsigned int count = 0;
            for (unsigned int i = 0; i < subtask.proc_num; i++) {
                if (trans_count[i] != -1) {
                    count += trans_count[i] + 2; // + 2 = 1 * 2 for the Subtask itself (weight == 2)
                }
            }
            return count;
        }();
        const int x = calculate_begin(subtask);
        const int y = elems_before * y_unit;
        const int width = calculate_width(subtask);
        const int height = y_unit * 2;
        const SDL_Rect rect{x, y, width, height};

        SDL_Surface*  surface = TTF_RenderText_Solid(font, subtask.name.c_str(), subtask_color);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surface);
        drawing_elements.emplace_back(std::move(rect), surface, texture, subtask_color);

        const auto& curr_transmissions = subtask.transmissions;
        for (unsigned int index = 0; index < curr_transmissions.size(); index++) {
        	const auto& curr_trans = curr_transmissions[index];
            const int x = calculate_begin(curr_trans);
            const int y = (elems_before + 2 + index) * y_unit; // + 2 = 1 * 2 for the Subtask itself (weight == 2)
            const int width = calculate_width(curr_trans);
            const int height = y_unit;
            const SDL_Rect rect{x, y, width, height};

            const std::string to_proc = std::string{"To "} + std::to_string(curr_trans.proc_dest) + " : " + subtask.name;
            SDL_Surface*  surface = TTF_RenderText_Solid(font, to_proc.c_str(), transmission_color);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surface);
        	drawing_elements.emplace_back(std::move(rect), surface, texture, transmission_color);
        }
        core_separators.push_back((elems_before + 2 + curr_transmissions.size()) * y_unit);
    }

    // Draw stuff
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        //Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            if ((e.type == SDL_QUIT) || (e.key.keysym.sym == SDLK_q)) {
                quit = true;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);

        // Draw a grid
        SDL_SetRenderDrawColor(gRenderer, 0xC0, 0xC0, 0xC0, 0xFF);
		for (unsigned int i = 0; i < SCREEN_HEIGHT; i += y_unit) {
			SDL_RenderDrawLine(gRenderer, 0, i, SCREEN_WIDTH, i);
		}
		for (unsigned int i = 0; i < SCREEN_WIDTH; i += x_unit) {
			SDL_RenderDrawLine(gRenderer, i, 0, i, SCREEN_HEIGHT);
		}
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xF0, 0xFF);
		for (unsigned int separator : core_separators) {
			for (int line = -2; line <= 2; line++) {
				SDL_RenderDrawLine(gRenderer, 0, separator + line, SCREEN_WIDTH, separator + line);
			}
		}

        if (!drawing_elements.empty()) {
        	for (const DrawingElement element : drawing_elements) {
        		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
            	SDL_RenderDrawRect(gRenderer, &element.rectangle);
            	SDL_RenderCopy(gRenderer, element.texture, nullptr, &element.rectangle);
        	}
        }
        // Update screen
        SDL_RenderPresent(gRenderer);
    }

    for (const DrawingElement element : drawing_elements) {
		SDL_DestroyTexture(element.texture);
    	SDL_FreeSurface(element.surface);
	}
    TTF_CloseFont(font);
    TTF_Quit();

    close();
}

// ========================================================= //


int main() {

    std::vector<Subtask> subtasks;
    subtasks.emplace_back(1, "A", 0, 2, std::vector<Transmission>{});
    subtasks.emplace_back(1, "B", 2, 4, std::vector<Transmission>{});

    std::vector<Transmission> transmissions1;
    transmissions1.emplace_back(Transmission(3, 4, 1));
    transmissions1.emplace_back(Transmission(3, 4, 3));

    subtasks.emplace_back(2, "C", 0, 3, transmissions1);

    std::vector<Transmission> transmissions2;
    transmissions2.emplace_back(5, 6, 1);

    subtasks.emplace_back(3, "D", 4, 5, transmissions2);
    subtasks.emplace_back(1, "E", 6, 7, std::vector<Transmission>{});

    for (auto& subtask : subtasks) {
        std::cout << subtask << std::endl;
    }

    drawGraph(subtasks);


    return 0;
}
