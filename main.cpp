#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <optional>


const unsigned int WIDTH = 640;
const unsigned int HEIGT = 480;


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


	// return {(unsigned int) WIDTH / max_x, (unsigned int) HEIGT / sum_y};
	return { max_x, sum_y};
}


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

	const std::pair<float, unsigned int> counted = getUnits(chunks);
	std::cout << "(width = " << counted.first << ", height = " << counted.second << ")" << std::endl;


	return 0;
}