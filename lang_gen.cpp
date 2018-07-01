#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <string>
#include <time.h>

#define RAND_MAX LONG_MAX
#define WORDLEN_MIN 3

/// Replicated n amounts of data from a given set of markov chains (a .mchain file)

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

std::vector<markov_chain> chains;

char chain_ext[] = ".mchain";
char vowels[] = "aeiou";

long word_count = 1;
std::fstream chain_file;
std::fstream out_file;

char invalid_size[] = "The given output data length is invalid.\n\n";

void helpMsg(char* filename) {
	std::cout << filename << " <word_count> <chain_file> <out_name>" << std::endl;
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
		if ((s.at(i) < 97 || s.at(i) > 122 ) && s.at(i) != 32) return false;
	}
	return true;
}

bool validChar(char c) {
	if ((c < 97 || c > 122 ) && c != 32) return false;
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

char chooseNext(const long index) {
	markov_chain chain = chains.at(index);
	if (chain.total_count == 0) {
		for (markov_chain::symbc next : chain.following) {
			chain.total_count += next.count;
		}
	}
	long lrand = rand() % chain.total_count;

	char nextc;
	for (unsigned char i = 0; i < chain.following.size() && lrand >= 0; i++) {
		lrand -= chain.following.at(i).count;
		nextc = chain.following.at(i).symb;
	}

	return nextc;
}

int main(int argc, char** argv) {
	srand(time(NULL));
	if (argc != 4) {
		helpMsg(argv[0]);
		return 1;
	} else {
		if(!strtolW(argv[1], word_count) || word_count < 1) {
			std::cout << invalid_size;
			helpMsg(argv[0]);
			return 1;
		}
		chain_file.open(argv[2], std::ios::in);
		out_file.open(argv[3], std::ios::out);
	}

	if (!chain_file.is_open()) {
		std::cout << "Could not open .mchain file!" << std::endl;
		return 1;
	}
	if (!out_file.is_open()) {
		std::cout << "Coulnd not open output file!" << std::endl;
		return 1;
	}

	std::string line;
	std::getline(chain_file, line);
	if (line.empty() || !strtolW((char*) line.c_str(), approx_order)) {
		lineDebug(line);
		return 1;
	}

	std::cout << "Order of approximation is " << std::to_string(approx_order) << std::endl;

	while (std::getline(chain_file, line)) {
		if (line.empty()) continue;

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
	chain_file.close();

	char* symbs = new char[approx_order+1];
	bool has_vowel = false;
	short word_len = 0;
	long wcount = 0;

	long cchain = rand() % chains.size();
	out_file.write(chains.at(cchain).symbs, approx_order);
	memcpy(symbs, chains.at(cchain).symbs, approx_order);
	while (wcount < word_count) {
		symbs[approx_order] = chooseNext(cchain);
		if (symbs[approx_order] == 32) {
			if (word_len < WORDLEN_MIN || !has_vowel) continue;
			has_vowel = false;
			word_len = 0;
			wcount++;
		} else {
			if(!has_vowel) {
				for (char v : vowels) {
					if (symbs[approx_order] == v) {
						has_vowel = true;
						break;
					}
				}	
			}
			word_len++;
		}
		out_file.put(symbs[approx_order]);
		memmove(symbs, symbs+1, approx_order);
		cchain = findChain(symbs);
		
		if (cchain == -1) {
			std::cout << "Guessing for: ";
			std::cout.write(symbs, approx_order);
			std::cout << std::endl;
			cchain = rand() % chains.size();
		}
	}

	out_file.close();

	delete[] symbs;
	for (markov_chain chain : chains) {
		delete[] chain.symbs;
	}
	return 0;
}