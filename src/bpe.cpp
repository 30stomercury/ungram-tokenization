#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <stack>


using namespace std;

// split words
vector<string> splitWords(string str, char splitter) {
    vector<string> v = {};
    string word = "";
    for (std::string::iterator it = str.begin(), end = str.end(); it != end; ++it) {
        if (*it == splitter) {
            v.push_back(word);
            word = "";
        }
        else {
            word = word + *it;
        }
    }
    v.push_back(word);

    return v;
}

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

string dfs(
    std::pair<string, string> mypair, 
    std::map<string, std::pair<string, string>>& reversed_merges,
    int init_vocab_size) {
    string output = "";
    std::stack<string> q;
    q.push(mypair.second);
    q.push(mypair.first);
    //cout << mypair.first << " " << mypair.second << endl;
    while (!q.empty()) {
        string ele = q.top();
        q.pop();
        //cout << ele << endl;
        if (std::stoi(ele) < init_vocab_size) {
            output += ele;
            if (q.size() != 0) {
                output += "_";
            }
        }
        else {
            auto p = reversed_merges[ele];
            q.push(p.second);
            q.push(p.first);
            //cout << p.first << " " << p.second << endl;
        }
    }
    //cout << output << endl;
    return output;
}

template<class T>
auto print_vec(vector<T> v) {
    cout << v[0];
    for (int i = 1; i < v.size(); i++) {
        cout << " " << v[i];
    }
    cout << endl;
}

int main(int argc, char* argv[]) {
    int init_vocab_size = 128;
    int vocab_size = 4096;
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
    auto single_freqs = compute_single_freq(utts);
    for (int i = 0; i < init_vocab_size; i++) {
        auto token = std::to_string(i);
        freqfile << token << " " << single_freqs[token] << endl;
    }

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
        auto seg = dfs(most_freq_pair, reversed_merges, init_vocab_size);
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
