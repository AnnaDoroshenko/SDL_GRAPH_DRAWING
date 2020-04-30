#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <optional>


const unsigned int WIDTH = 640;
const unsigned int HEIGT = 480;


struct Transmission {
	public:
		float begin_at;
		float finish_at;
		unsigned int proc_dest;

	public:
		Transmission(const float& begin_at, const float& finish_at, 
					const unsigned int& proc_dest) : begin_at(begin_at), 
					finish_at(finish_at), proc_dest(proc_dest) {}

		friend std::ostream& operator<<(std::ostream& os, const Transmission& transmission);
};


struct Chunk {
	public:
		unsigned int proc_num;
		std::string name;
		float begin_at;
		float finish_at;
		std::optional<std::vector<Transmission>> transmissions;

	public:
		Chunk(const unsigned int& proc_num, const std::string& name,
			const float& begin_at, const float& finish_at,
			const std::optional<std::vector<Transmission>>& transmissions) :
			proc_num(proc_num), name(name), begin_at(begin_at),
			finish_at(finish_at), transmissions(transmissions) {}

		friend std::ostream& operator<<(std::ostream& os, const Chunk& chunk);
};


std::ostream& operator<<(std::ostream& os, const Transmission& trans) {
	os << " T(b: " << trans.begin_at << ", f: " << trans.finish_at << ", dest: " << trans.proc_dest << ")";
}


std::ostream& operator<<(std::ostream& os, const Chunk& chunk) {
	os << "Chunk(proc: " << chunk.proc_num << ", name: " << chunk.name << ", b: " << chunk.begin_at
		<< ", f: " << chunk.finish_at << ", transmissions:";

	if (chunk.transmissions) {
		for (unsigned int i = 0; i < chunk.transmissions->size(); i++)
        {
            os << (*chunk.transmissions)[i] << "";
        }
	} else {
		os << " none";
	}
	os << ")";
}

std::pair<float, unsigned int> getUnits(const std::vector<Chunk>& chunks) {
	// calculation of x_unit
	float max_x = 0;
	for (auto& chunk : chunks) {
		float curr_max_time = chunk.finish_at;
		if (chunk.transmissions) {
			curr_max_time = (*chunk.transmissions).back().finish_at;
		}
		if (curr_max_time > max_x) max_x = curr_max_time;
	}

	// calculation of y_unit
	unsigned int sum_y = 0;
	for (auto& chunk : chunks) {
		const unsigned int curr_proc_num = chunk.proc_num;
		unsigned int trans_count = 0;
		for (auto& ch : chunks) {
			if (curr_proc_num != ch.proc_num) continue;
			const unsigned int curr_trans_size = ch.transmissions->size();
			if (curr_trans_size > trans_count) trans_count = curr_trans_size;
		}
		// + 1 for the main chunk
		sum_y += trans_count + 1;
	}


	// return {(unsigned int) WIDTH / max_x, (unsigned int) HEIGT / sum_y};
	return { max_x, sum_y};
}


int main() {
	std::vector<Chunk> chunks;
	chunks.push_back(Chunk(1, "A", 0, 2, std::nullopt));
	chunks.push_back(Chunk(1, "B", 2, 4, std::nullopt));

	std::vector<Transmission> transmissions1;
	transmissions1.push_back(Transmission(1.5, 3, 1));
	transmissions1.push_back(Transmission(1.5, 3, 3));

	chunks.push_back(Chunk(2, "C", 0, 1.5, transmissions1));

	std::vector<Transmission> transmissions2;
	transmissions2.push_back(Transmission(5, 6, 1));

	chunks.push_back(Chunk(3, "D", 4, 5, transmissions2));
	chunks.push_back(Chunk(1, "E", 6, 7.5, std::nullopt));

	for (auto& chunk : chunks) {
		std::cout << chunk << std::endl;
	}

	const std::pair<float, unsigned int> counted = getUnits(chunks);
	std::cout << "(width = " << counted.first << ", height = " << counted.second << ")" << std::endl;


	return 0;
}