#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <string>

/// Creates a file with markov chains of a specific order given a dataset,
/// data can be appended to an existing .mchain file using the -a flag

long approx_order = 1;

class markov_chain {
	public:
	long total_count = 0;
	char* symbs;
	struct symbc {
		char symb;
		long count;
	};
	std::vector<symbc> following;
	void add(const char symbol) {
		for (symbc &next : following) {
			if (next.symb == symbol) {
				next.count++;
				return;
			}
		}
		following.push_back({symbol, 1});
	}
	void set(const char symbol, const long count) {
		for (symbc &next : following) {
			if (next.symb == symbol) {
				next.count = count;
				return;
			}
		}
		following.push_back({symbol, count});
	}
	markov_chain(char* symbs) : symbs(new char[approx_order]) { 
		std::copy(symbs, symbs+approx_order, this->symbs);
	}
	~markov_chain() {}
};

bool chain_append = false;
const char chain_ext[] = ".json";
std::string ofile_name;
std::fstream out_file;
std::ifstream data_file;

std::vector<markov_chain> chains;


void helpMsg(char* filename) {
	std::cout << filename << " [-a | <order_of_approximation>] <data_file> <out_name>" << std::endl;
}

bool strtolW(const char* str, long& num) {
	char* bad_c;
	num = strtol(str, &bad_c, 0);
	if (bad_c == nullptr) {
		return 0;
	} else return 1;
}

bool validChars(std::string s) {
	for (int i = 0; i < s.length(); i++) {
		if ((s.at(i) < 97 || s.at(i) > 122) && s.at(i) != 32) return false;
	}
	return true;
}

bool validChar(char c) {
	if ((c < 97 || c > 122) && c != 32) return false;
	else return true;
}

void lineDebug(const std::string l) {
	std::cout << "Invalid Syntax for line: '" << l << "'" << std::endl;
}

long findChain(const char* str) {
	for (long i = 0; i < chains.size(); i++) {
		if (memcmp(chains.at(i).symbs, str, approx_order) == 0) {
			return i;
		}
	}
	return -1;
}

int main(int argc, char** argv) {
	if (argc > 5 || argc < 4) {
		helpMsg(argv[0]);
		return 1;
	} else {
		ofile_name.assign(argv[3]);
		if (ofile_name.find(chain_ext) == std::string::npos) {
			ofile_name.append(chain_ext);
		}
		if (memcmp(argv[1], "-a", 2) == 0) {
			chain_append = true;
			out_file.open(ofile_name, std::ios::in);
		} else {
			if (!strtolW(argv[1], approx_order) || approx_order < 1 || approx_order > 10) {
				std::cout << "The order of approximation is invalid." << std::endl << std::endl;
				helpMsg(argv[0]);
				return 1;
			}
			out_file.open(ofile_name, std::ios::out);
		}
	}
	data_file.open(argv[2]);

	if (!data_file.is_open()) {
		std::cout << "Unable to open data file!" << std::endl;
		return 1;
	}
	if (!out_file.is_open()) {
		std::cout << "Unable to open output file!" << std::endl;
		return 1;
	}

	if (chain_append) { //read chain data
		std::cout << "Appending data to existing chain file!" << std::endl;
		std::string line;
		std::getline(out_file, line);
		if (line.empty() || !strtolW((char*) line.c_str(), approx_order)) {
			lineDebug(line);
			return 1;
		}

		std::cout << "Order of approximation is " << std::to_string(approx_order) << std::endl;

		while (std::getline(out_file, line)) {
			if (line.empty()) continue;
			if (!validChars(line)) {
				std::cout << "Invalid characters in line!" << std::endl;
				lineDebug(line);
				return 1;
			}

			char* si = (char*) line.c_str();
			char* ni;
			long cchain = chains.size();
			chains.push_back(markov_chain(si));

			si += approx_order + 1;
			ni = strchr(si, ',');
			while (ni != nullptr) {
				ni[0] = 0;
				long num;
				if (!strtolW(si+2, num)) {
					lineDebug(line);
					return 1;
				}
				chains.at(cchain).set(si[0], num);
				si = ni+1;
				ni = strchr(si, ',');
			}
		}
		out_file.close();
		out_file.open("temp.mchain", std::ios::out | std::ios::trunc);
	}

	char* symbs = new char[approx_order+1];
	unsigned char ccount = 0;
	char cc;
	while (data_file.get(cc)) { // fill characters needed for order of chain
		if (cc == '\n' || cc == '\r') {
			if (ccount != 0 && symbs[ccount-1] != ' ') cc = ' ';
		}
		if (!validChar(cc)) continue;
		symbs[ccount] = cc;
		ccount++;
		if (ccount == approx_order) {
			break;
		}
	}
	if (ccount != approx_order) {
		std::cout << "Data in file is not large enough for given order of approximation." << std::endl;
		return 1;
	}
	while (data_file.get(symbs[approx_order])) {
		if ((symbs[approx_order] == '\n' || symbs[approx_order] == '\r') && symbs[approx_order-1] != ' ') symbs[approx_order] = ' ';
		if (!validChar(symbs[approx_order])) continue;
		long cchain = findChain(symbs);
		if (cchain == -1) {
			cchain = chains.size();
			chains.push_back(markov_chain(symbs));
		}
		chains.at(cchain).add(symbs[approx_order]);
		memmove(symbs, symbs+1, approx_order);
	}
	delete[] symbs;
	data_file.close();

	//write chain data to file
	out_file << "{\n\"predict\":" << std::to_string(approx_order).c_str() << ",\n";

	for (long i = 0; i < chains.size(); i++) {
		markov_chain chain = chains.at(i);
		out_file.put('"');
		out_file.write(chain.symbs, approx_order);
		out_file.write("\":{", 3);
		int len = chain.following.size();
		for (int j = 0; j < len; j++) {
			markov_chain::symbc next = chain.following.at(j);
			out_file.put('"');
			out_file.put(next.symb);
			out_file.write("\":",2);
			out_file << std::to_string(next.count).c_str();
			if (j != len-1) out_file.put(',');
		}
		out_file.write("},",2);
		if (i != chains.size()-1) out_file.put('\n');
	}
	out_file << "\n}";
	out_file.close();

	if (chain_append) {
		if (remove(ofile_name.c_str()) != 0) {
			std::cout << "Error deleting outdated chain file, please continue manually." << std::endl;
			return 1;
		}
		if (rename("temp.mchain", ofile_name.c_str()) != 0) {
			std::cout << "Error renaming temp file, please continue manually." << std::endl;
			return 1;
		}
	}
	
	for (markov_chain chain : chains) {
		delete[] chain.symbs;
	}
	return 0;
}