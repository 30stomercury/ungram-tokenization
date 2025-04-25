#include<vector>
#include<string>
#include <tuple>
#include <map>
#include <stack>
#include <cmath>
#include <algorithm>


#ifndef UNIGRAM_H
#define UNIGRAM_H
struct Node {
    std::map<std::string, Node*> child;
    bool isword;
    double logprob;
};

Node* buildTrie(const std::map<std::vector<std::string>, double>& freqs);

std::pair<bool, double> isWord(const std::vector<std::string>& subword, Node* trie);

template<class T>
auto print_vec(std::vector<T> v);

std::vector<std::string> splitWords(std::string str, char splitter);

std::tuple<std::vector<std::pair<int, int>>, std::vector<double>> 
viterbiForward(std::vector<std::string>& seq, Node* trie, int max_seg_len);

std::tuple<std::vector<std::string>, std::vector<std::vector<std::string>>>
viterbiBackward(const std::vector<std::string>& seq, 
                std::map<std::vector<std::string>, std::string>& vocab, 
                const std::vector<std::pair<int, int>>& backptr);

std::map<std::vector<std::string>, double> 
pruneVocab(const std::map<std::vector<std::string>, double>& freqs, 
        std::map<std::vector<std::string>, std::string>& vocab, int init_vocab_size, 
        int target_vocab_size, Node* trie);

std::string dfs(
    const std::pair<std::string, std::string> mypair, 
    const std::map<std::string, std::pair<std::string, std::string>>& reversed_merges,
    const std::map<std::string, int> single_freqs);

std::string concatVec(std::vector<std::string> v, std::string splitter);

std::string concatVecOfVec(std::vector<std::vector<std::string>> v, std::string splitter);
#endif
