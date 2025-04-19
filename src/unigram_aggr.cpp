#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "unigram.h"


using namespace std;



int main(int argc, char* argv[]) {
    int split = std::stoi(argv[1]);
    vector<string> v;
    string text;
    std::map<vector<string>, double> freqs;
    std::map<vector<string>, string> vocab;
    std::map<vector<string>, string> vec2string;
    
    // load freqs, vocab
    string freqsname = argv[2]; 
    ifstream freqsFile;
    for (int i = 1; i <= split; ++i) {
        auto infile = freqsname + "." + std::to_string(i) + ".freq";
        cout << infile << endl;
        freqsFile.open(infile, ios::in);
        while (getline(freqsFile, text)) {
            v = splitWords(text, ' ');
            auto subword = splitWords(v[0], '_');
            freqs[subword] += std::stol(v[1]);
            //cout << text << endl;
        }
        freqsFile.close();
    }

    // save aggregated freqs
    cout << freqsname + ".freq" << endl;
    freqsFile.open(freqsname + ".1.freq", ios::in);
    ofstream outFile(freqsname + ".freq");
    int vocab_id = 0;
    while (getline(freqsFile, text)) {
        v = splitWords(text, ' ');
        auto subword = splitWords(v[0], '_');
        long count = (long) freqs[subword];
        outFile << v[0] << " " << count << endl;
        vocab[subword] = std::to_string(vocab_id);
        vec2string[subword] = v[0];
        vocab_id += 1;
    }
    freqsFile.close();
    outFile.close();



    int init_vocab_size = 128;
    float reduce_rate = std::stof(argv[3]);
    int target_vocab_size = std::stoi(argv[4]);
    int curr_vocab_size = vocab_id;

    if (reduce_rate < 1.0 && curr_vocab_size > target_vocab_size) {
        curr_vocab_size = (int) vocab_id * reduce_rate;
        curr_vocab_size = std::max(curr_vocab_size, target_vocab_size);
        cout << "Pruning vocab from " << vocab_id << " to " << curr_vocab_size << endl;
        // build trie
        auto trie = buildTrie(freqs);

        // pruning
        auto pruned_freqs = pruneVocab(freqs, vocab, init_vocab_size, curr_vocab_size, trie);

        // save
        outFile.open(freqsname + "_pruned.freq");
        long count;
        for (auto const& item: pruned_freqs) {
            count = (long) item.second;
            outFile << vec2string.at(item.first) << " " << std::fixed << count << endl;
        }
        outFile.close();
    }
}
