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
    std::map<vector<string>, string> vec2string;
    
    // load freqs, vocab
    string freqsname = argv[2]; 
    ifstream freqsFile;
    for (int i = 1; i <= split; ++i) {
        auto infile = freqsname + "." + std::to_string(i) + ".freq";
        if (split == 1) {
            infile = freqsname + ".freq";
        }
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
    std::map<vector<string>, string> vocab;
    cout << freqsname + ".freq" << endl;
    ofstream outFile;
    auto infile = freqsname + ".freq";
    if (split > 1) {
        infile = freqsname + ".1.freq";
        outFile.open(freqsname + ".freq");
    }
    freqsFile.open(infile, ios::in);
    int vocab_id = 0;

    while (getline(freqsFile, text)) {
        v = splitWords(text, ' ');
        auto subword = splitWords(v[0], '_');
        long count = (long) freqs[subword];

        if (split > 1) {
            outFile << v[0] << " " << count << endl;
        }

        vocab[subword] = std::to_string(vocab_id);
        vec2string[subword] = v[0];
        vocab_id += 1;
    }
    freqsFile.close();
    if (split > 1) {
        outFile.close();
    }


    float reduce_rate = std::stof(argv[3]);
    int init_vocab_size = std::stoi(argv[4]);
    int target_vocab_size = std::stoi(argv[5]);
    int curr_vocab_size = vocab_id;
    ofstream prunedFile;

    cout << curr_vocab_size << " " << target_vocab_size << endl;

    if (reduce_rate < 1.0 && curr_vocab_size > target_vocab_size) {
        curr_vocab_size = (int) vocab_id * reduce_rate;
        curr_vocab_size = std::max(curr_vocab_size, target_vocab_size);
        cout << "Pruning vocab from " << vocab_id << " to " << curr_vocab_size << endl;
        // build trie
        auto trie = buildTrie(freqs);

        // pruning
        auto pruned_freqs = pruneVocab(freqs, vocab, init_vocab_size, curr_vocab_size, trie);

        // save
        prunedFile.open(freqsname + "_pruned.freq");
        long count;
        for (auto const& item: pruned_freqs) {
            count = (long) item.second;
            prunedFile << vec2string.at(item.first) << " " << std::fixed << count << endl;
        }
        prunedFile.close();
    }
}
