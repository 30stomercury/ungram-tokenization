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


std::map<vector<string>, long> decodeFile(const string& inputFilename, const string& outputFilename, 
                 Node* trie, int max_subword_len, std::map<vector<string>, string>& vocab) {

    ifstream unitFile(inputFilename);
    if (!unitFile.is_open()) {
        cerr << "Error: Unable to open file: " << inputFilename << endl;
        unitFile.close();
    }
    ofstream outFile(outputFilename);
    if (!outFile.is_open()) {
        cerr << "Error: Unable to open file: " << outputFilename << endl;
        outFile.close();
    }
    ofstream outSegFile(outputFilename+".seg");
    if (!outSegFile.is_open()) {
        cerr << "Error: Unable to open file: " << outputFilename + ".seg" << endl;
        outSegFile.close();
    }

    std::map<vector<string>, long> est_freqs;
    for (auto const& item: vocab) {
        est_freqs[item.first] = 0;
    }
    int num_segs = 0;
    double nll = 0.0;
    string text;
    int num_line = 0;

    while (getline(unitFile, text)) {
        if (num_line % 3 == 1) {
            outFile << endl;
            outSegFile << endl;
            vector<string> input = splitWords(text, ' ');
            auto results = viterbiForward(input, trie, max_subword_len + 1);
            auto outputs = viterbiBackward(input, vocab, get<0>(results));
            vector<string> decoded_seq = get<0>(outputs);
            vector<vector<string>> decoded_segments = get<1>(outputs);
            string decoded_string = concatVec(decoded_seq, " ");
            outFile << decoded_string;
            string decoded_seg_string = concatVecOfVec(decoded_segments, " | ");
            outSegFile << decoded_seg_string;

            // Compute NLL
            nll += get<1>(results).back();
            num_segs += decoded_seq.size();
            for (const auto& seg: decoded_segments) {
                est_freqs[seg] += 1;
            }
        }
        else if (num_line % 3 == 0) {
            if (num_line != 0) {
                outFile << '\n';
                outSegFile << '\n';
            }
            outFile << text;
            outSegFile << text;
        }
        else if (num_line % 1 == 0) {
            outFile << '\n' << text;
            outSegFile << '\n' << text;
        }
        num_line += 1;
    }

    unitFile.close();
    outFile.close();
    outSegFile.close();

    // Save log
    ofstream logFile(outputFilename + ".log");
    logFile << "total loss: " << nll << ", " 
        << "total segments: " << num_segs << ", "
        << "ave loss: " << nll / num_segs << endl;
    logFile.close();

    return est_freqs;
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


void saveFreqs(const string& freqsFilename, const string& estfreqsFilename, 
                          int vocab_size, const map<vector<string>, long>& est_freqs) {
    ifstream freqsFile(freqsFilename);
    ofstream estfreqsFile(estfreqsFilename);

    string text;
    int vocab_id = 0;

    while (getline(freqsFile, text) && vocab_id < vocab_size) {
        vector<string> v = splitWords(text, ' ');
        vector<string> subword = splitWords(v[0], '_');

        // Write the estimated frequency to the file
        estfreqsFile << v[0] << " " << est_freqs.at(subword) << endl;

        vocab_id += 1;
    }

    freqsFile.close();
    estfreqsFile.close();
}



int main(int argc, char* argv[]) {
    int vocab_size = std::stoi(argv[3]);
    vector<string> v;
    string text;
    std::map<vector<string>, string> vocab;
    std::map<vector<string>, double> freqs;
    string freqsname = argv[4]; 
    
    // load freqs
    cout << freqsname + ".freq" << endl;
    int max_subword_len = loadFreqs(freqsname + ".freq", vocab_size, vocab, freqs);

    // build trie
    auto trie = buildTrie(freqs);

    // decode
    // infile: argv[1], outfile: argv[2]
    auto est_freqs = decodeFile(argv[1], argv[2], trie, max_subword_len, vocab);

    // Save est freqs
    string estfreqsname = argv[2];
    saveFreqs(freqsname + ".freq", estfreqsname + ".freq", vocab_size, est_freqs);
}
