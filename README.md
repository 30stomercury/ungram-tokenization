## Unigram tokenization in C++

This is an implementation of unigram tokenization in C++ (for fun). In particular, it doesn't treat whitespace as a delimiter between text segments.

More details can be found in the original paper:
- [Subword Regularization: Improving Neural Network Translation Models with Multiple Subword Candidates](https://aclanthology.org/P18-1007/)


### Compile
```
mkdir bin
g++ src/bpe.cpp src/unigram.cpp -o bin/bpe.out
g++ src/decode.cpp src/unigram.cpp -o bin/unigram_decode.out
g++ src/unigram_aggr.cpp src/unigram.cpp -o bin/unigram_aggr.out
```

### Run

The training consists of two steps.

1. Collecting frequent substrings with BPE.
```
bin/bpe.out input_text_file output init_vocab_size target_vocab_size
```
2. Initialize the unigram distribution with frequent substrings, and update the distribution with EM algorithm and 
iterative vocab refinements.
```
mkdir out_dir
./train_parallel.sh input_text_file out_dir target_vocab_size min_vocab_size output.freq
```
- If your data is not too large:
```
./train.sh input_text_file out_dir target_vocab_size min_vocab_size output.freq
```


### Example
```
mkdir exp
bin/bpe.out test.txt test 39 256
./train.sh test.txt exp 128 39 test.freq
```
