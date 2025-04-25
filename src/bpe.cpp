#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <stack>
#include <cassert>
#include "unigram.h"


using namespace std;


std::map<string, int> compute_single_freq(vector<vector<string>>& v) {
    std::map<string, int> freqs;
    for (int i = 0; i < v.size(); i++) {
        auto& vi = v[i];
        int j = 0;
        while (j < vi.size()) {
            freqs[vi[j]] += 1;
            j++;
        }
    }
    return freqs;
}

std::map<std::pair<string, string>, int> compute_pair_freq(vector<vector<string>>& v) {
    std::map<std::pair<string, string>, int> freqs;
    for (int i = 0; i < v.size(); i++) {
        auto& vi = v[i];
        int j = 0;
        while (j < vi.size() - 1) {
            freqs[{vi[j], vi[j + 1]}] += 1;
            j++;
        }
    }
    return freqs;
}

void merge_pairs(
    vector<vector<string>>& v, 
    std::pair<string, string> merge_pair,
    std::map<std::pair<string, string>, string>& merges) {
    for (int i = 0; i < v.size(); i++) {
        auto& vi = v[i];
        int j = 0;
        vector<string> new_split;
        while (j < vi.size() - 1) {
            if (vi[j] == merge_pair.first && vi[j + 1] == merge_pair.second) {
                // Add the merged pair
                new_split.push_back(merges[merge_pair]);
                j += 2;  // Skip the next token since it's merged
            } 
            else {
                new_split.push_back(vi[j]);
                j++;
            }
        }
        // If the last element was not included
        if (j == vi.size() - 1) {
            new_split.push_back(vi.back());
        }

        // Update the splits for the word
        v[i] = new_split;
    }
}


int main(int argc, char* argv[]) {
    vector<string> v;
    vector<vector<string>> utts;
    string text;
    std::map<std::pair<string, string>, string> merges;
    std::map<string, std::pair<string, string>> reversed_merges;
    std::map<std::pair<string, string>, int> freqs;
    
    string filename = argv[1];
    ifstream wrdFile;
    wrdFile.open(filename, ios::in);
    int num_line = 0;
    string utt;
    vector<string> new_v;
    while (getline(wrdFile, text)){
        v = splitWords(text, ' ');
        utts.push_back(v);
    }
    wrdFile.close();

    // output
    filename = argv[2];
    ofstream freqfile;
    freqfile.open(filename + ".freq");
    ofstream mergefile;
    mergefile.open(filename + ".model");

    // count individual freqs
    int init_vocab_size = std::stoi(argv[3]);
    int vocab_size = std::stoi(argv[4]);

    auto single_freqs = compute_single_freq(utts);
    for (auto const& item: single_freqs) {
        freqfile << item.first << " " << item.second << endl;
    }
    assert(init_vocab_size >= single_freqs.size());

    for (int i = init_vocab_size; i < vocab_size; i++) {
        //  count pair freqs
        auto freqs = compute_pair_freq(utts);
        int max = 0;
        std::pair<string, string> most_freq_pair;
        for (auto const& item: freqs) {
            if (max < item.second) {
                max = item.second;
                most_freq_pair = item.first;
            }
        }

        // merges
        merges[most_freq_pair] = std::to_string(i);
        reversed_merges[std::to_string(i)] = most_freq_pair;
        auto seg = dfs(most_freq_pair, reversed_merges, single_freqs);
        freqfile << seg << " " << max << endl;
        mergefile << most_freq_pair.first << " " << most_freq_pair.second
             << " " << i << endl;
        cout << "merging token" << " " 
             << i << endl;
        merge_pairs(utts, most_freq_pair, merges);
    }
    freqfile.close();
    mergefile.close();
}
