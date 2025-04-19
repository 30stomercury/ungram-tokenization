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
#include <assert.h>



using namespace std;

double inf = numeric_limits<double>::infinity();

bool myComparison(
    const std::pair<vector<string>, double> &a,
    const std::pair<vector<string>,double> &b) {
    return a.second < b.second;
}

struct Node {
    std::map<string, Node*> child;
    bool isword;
    double logprob;
};

Node* buildTrie(const std::map<vector<string>, double>& freqs) {
    Node* root = new Node();
    double sum_counts;
    for (auto const& item: freqs) {
        sum_counts += item.second;
    }

    for (auto const& item: freqs) {
        auto subword = item.first;
        Node* temp = root;
        for (auto const ele : subword) {
            if (!temp->child[ele]) {
                temp->child[ele] = new Node();
            }
            temp = temp->child[ele];
        }
        temp->isword = true;
        temp->logprob = std::log(item.second / sum_counts);
    }
    return root;
}

Node* getTail(const vector<string>& subword, Node* trie) {
    Node* tail = trie;
    for (auto const w: subword) {
        if (!tail->child[w]) {
            return nullptr;
        }
        tail = tail->child[w];
    } 
    return tail;
}

std::pair<bool, double> isWord(const vector<string>& subword, Node* trie) {
    Node* tail = getTail(subword, trie);
    if (tail == nullptr) {
        return {false, 0.0};
    }
    return {tail->isword, tail->logprob};
}

std::tuple<vector<std::pair<int, int>>, vector<double>> 
viterbiForward(vector<string>& seq, Node* trie, int max_seg_len) {
    vector<std::pair<int, int>> backptr(seq.size() + 1);
    vector<double> nll(seq.size() + 1);
    for (int end = 1; end <= seq.size(); end++) {
        nll[end] = inf;
        int window = std::min(max_seg_len, end);
        for (int start=end-window; start < end; start++) {
            //cout << start << " " << end << endl;
            vector<string> subvector = {
                seq.begin() + start, 
                seq.begin() + end
            }; 
            // {isword, logprob}
            auto res = isWord(subvector, trie);
            if (res.first) {
                double score = nll[start] - res.second;   
                if (score < nll[end]) {
                    nll[end] = score; 
                    backptr[end] = {start, end};
                }
            }
        }
    }
    return {backptr, nll};
}

std::tuple<vector<string>, vector<vector<string>>> 
viterbiBackward(const vector<string>& seq, 
                std::map<vector<string>, string>& vocab, 
                const vector<std::pair<int, int>>& backptr) {
    vector<string> subwords;
    vector<vector<string>> segments;

    int next_seg_index = backptr.size() - 1;
    int total_len = 0;

    while (total_len != seq.size()) {
        int start = backptr[next_seg_index].first;
        int end = backptr[next_seg_index].second;
        total_len += (end - start);
        next_seg_index = backptr[next_seg_index].first;
        vector<string> subword = {seq.begin() + start, seq.begin() + end}; 
        subwords.push_back(vocab[subword]);
        segments.push_back(subword);
    }

    std::reverse(subwords.begin(), subwords.end());
    std::reverse(segments.begin(), segments.end());

//    string result = "";
//    int i = 0;
//    for (const auto& subword : subwords) {
//        result += subword;
//        if (i < subwords.size() - 1) {
//            result += " ";
//        }
//        i++;
//    }

    return {subwords, segments};
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

std::map<vector<string>, double> 
pruneVocab(
    const std::map<vector<string>, double>& freqs, 
    std::map<vector<string>, string>& vocab, int init_vocab_size, int next_vocab_size, Node* trie) {

    assert(next_vocab_size >= init_vocab_size);


    std::map<vector<string>, double> pruned_freqs; 
    double sum_counts;
    std::map<vector<string>, double> logprobs;
    vector<std::pair<vector<string>, double>> loss;

    // make logprob
    for (auto const& item: freqs) {
        sum_counts += item.second;
    }
    for (auto const& item: freqs) {
        logprobs[item.first] = std::log(item.second / sum_counts);
    }

    for (auto const& item: freqs) {
        vector<string> subword = item.first;
        if (subword.size() == 1) { 
            loss.push_back({item.first, -inf});
        }
        if (subword.size() > 1) { 
            Node* tail = getTail(subword, trie);
            tail->isword = false;
            auto results = viterbiForward(subword, trie, subword.size());
            auto outputs = viterbiBackward(subword, vocab, get<0>(results));
            double nll = get<1>(results).back();
            tail->isword = true;
            double org_loss = freqs.at(subword) * tail->logprob;
            double new_loss = -1 * freqs.at(subword) * nll;
            //diff = new_loss - org_loss;
            double diff = freqs.at(subword) * (-nll + tail->logprob);
            if (freqs.at(subword) == 0) {
                diff = inf;
            }
            // the lower the better
            loss.push_back({subword, diff});
        }
    }
    sort(loss.begin(), loss.end(), myComparison);
    // prune
    loss = {loss.begin(), loss.begin() + next_vocab_size,}; 
    for (auto const& item: loss) {
        pruned_freqs[item.first] = freqs.at(item.first);
    }
    return pruned_freqs;
}


template<class T>
auto print_vec(vector<T> v) {
	cout << v[0];
    for (int i = 1; i < v.size(); i++) {
		cout << " " << v[i];
	}
	cout << endl;
}

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


string concatVec(vector<string> v, string splitter) {
    string result = "";
    int i = 0;
    for (const auto& vi : v) {
        result += vi;
        if (i < v.size() - 1) {
            result += splitter;
        }
        i++;
    }
    return result;
}


string concatVecOfVec(vector<vector<string>> v, string splitter) {
    string result = "";
    int i = 0;
    for (const auto& vi : v) {
        result += concatVec(vi, " ");
        if (i < v.size() - 1) {
            result += splitter;
        }
        i++;
    }
    return result;
}
