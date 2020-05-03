#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <SDL2/SDL.h>
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

    // Set texture filtering to linear
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
        std::cout << "Warning: Linear texture filtering not enabled!" << std::endl;
    }

    // Create window
    gWindow = SDL_CreateWindow("Little SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer for window
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == NULL) {
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
            sum_y += trans_count + 1; // + 1 for the Subtask itself
        }
    }


    return DrawingBasics({SCREEN_WIDTH / max_x, SCREEN_HEIGHT / sum_y},
            trans_count_max);
}


void drawGraph(const std::vector<Subtask>& subtasks) {
    const auto& [units, trans_count] = getDrawingBasics(subtasks);
    const auto& [x_unit, y_unit] = units;
    std::cout << "(x_unit = " << x_unit << ", y_unit = " << y_unit << ")" << std::endl;

    // Prepare stuff to draw
    std::vector<SDL_Rect> rectangles;
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
                    count += trans_count[i] + 1; // +1 for the Task itself
                }
            }
            return count;
        }();
        const int x = calculate_begin(subtask);
        const int y = elems_before * y_unit;
        const int width = calculate_width(subtask);
        const int height = y_unit;
        const SDL_Rect rect{x, y, width, height};
        rectangles.emplace_back(std::move(rect));

        const auto& curr_transmissions = subtask.transmissions;
        if (curr_transmissions.empty()) continue;
        for (unsigned int index = 0; index < curr_transmissions.size(); index++) {
            const int x = calculate_begin(curr_transmissions[index]);
            const int y = (elems_before + 1 + index) * y_unit; // +1 for the Task itself
            const int width = calculate_width(curr_transmissions[index]);
            const int height = y_unit;
            const SDL_Rect rect{x, y, width, height};
            rectangles.emplace_back(std::move(rect));
        }
    }

    if (!init()) {
        std::cout << "Failed to initialize!" << std::endl;
        return;
    }

    // Draw stuff
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        //Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // if (e.type == SDL_QUIT) {
            if ((e.type == SDL_QUIT) || (e.key.keysym.sym == SDLK_q)) {
                quit = true;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);

        if (!rectangles.empty()) {
            SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
            SDL_RenderDrawRects(gRenderer, &rectangles[0], rectangles.size());
        }

        // Update screen
        SDL_RenderPresent(gRenderer);
    }

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

    // const std::pair<float, unsigned int> counted = getUnits(Subtasks);
    // std::cout << "(width = " << counted.first << ", height = " << counted.second << ")" << std::endl;
    drawGraph(subtasks);


    return 0;
}
