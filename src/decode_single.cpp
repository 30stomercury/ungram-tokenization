#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <stack>
#include <cmath>
#include <algorithm>
#include <limits>
#include "unigram.h"



using namespace std;


void decodeSingle(string& input, Node* trie, 
        int max_subword_len, std::map<vector<string>, string>& vocab) {

    int num_segs = 0;
    double nll = 0.0;
    string text;
    int num_line = 0;

    vector<string> input_split = splitWords(input, ' ');
    auto results = viterbiForward(input_split, trie, max_subword_len + 1);
    auto outputs = viterbiBackward(input_split, vocab, get<0>(results));
    vector<string> decoded_seq = get<0>(outputs);
    vector<vector<string>> decoded_segments = get<1>(outputs);
    string decoded_string = get<2>(outputs);
    cout << decoded_string << endl;

    // Compute NLL
    nll += get<1>(results).back();
    num_segs += decoded_seq.size();
}

int loadFreqs(const string& freqsFilename, int vocab_size, 
                     map<vector<string>, string>& vocab,
                     map<vector<string>, double>& freqs) {

    ifstream freqsFile(freqsFilename);
    if (!freqsFile.is_open()) {
        cerr << "Error: Unable to open file: " << freqsFilename << endl;
        freqsFile.close();
    }

    string text;
    int max_subword_len = 0;
    int vocab_id = 0;

    while (getline(freqsFile, text) && vocab_id < vocab_size) {
        vector<string> v = splitWords(text, ' ');
        vector<string> subword = splitWords(v[0], '_');

        vocab[subword] = std::to_string(vocab_id);
        freqs[subword] = std::stod(v[1]);

        vocab_id += 1;

        // Update max_subword_len if needed
        if (max_subword_len < subword.size()) {
            max_subword_len = subword.size();
        }
    }

    freqsFile.close();
    return max_subword_len;
}



int main(int argc, char* argv[]) {
    int init_vocab_size = 128;
    int vocab_size = std::stoi(argv[2]);
    vector<string> v;
    string text;
    std::map<vector<string>, string> vocab;
    std::map<vector<string>, double> freqs;
    string freqsname = argv[3]; 
    
    // load freqs
    cout << freqsname + ".freq" << endl;
    int max_subword_len = loadFreqs(freqsname + ".freq", vocab_size, vocab, freqs);

    // build trie
    auto trie = buildTrie(freqs);

    // decode
    // infile: argv[1], outfile: argv[2]
    string input = argv[1];
    decodeSingle(input, trie, max_subword_len, vocab);
}
