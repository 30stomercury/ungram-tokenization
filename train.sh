set -ex

infilename=$1 # input file
path=$2 # output path
target_vocab_size=$3
min_vocab_size=$4 # canonical set of vocabs
init_subwords=$5 # init substrings: xxx.freq
n_epoch=1
reduce_rate=0.7
name=test
vocab_size=$(wc -l < $init_subwords)
outname=$name-viterbi-$vocab_size

echo $(basename $infilename)


# split
mkdir -p $path/log

while [ "$vocab_size" -gt $target_vocab_size ]
do
    echo "The current vocab size $vocab_size, init with $init_subwords, the target vocab size is $target_vocab_size"
    # EM
    vocab_size=$(wc -l < $init_subwords)
    outname=$name-viterbi-$vocab_size
    cp $init_subwords $path/$outname-e0.freq
    for e in `seq 1 $n_epoch`; do
        # E step
        ./bin/unigram_decode.out $infilename $path/$outname-e$e $vocab_size $path/$outname-e$((e-1))

        # keep log
        if [[ "$vocab_size" != "$target_vocab_size" || "$e" != "$n_epoch" ]]; then
            rm $path/$outname-e$e
            rm $path/$outname-e$e.seg
        fi

        # M step
        # ./bin/unigram_aggr.out n_split output reduce_rate min_vocab_size current_vocab_size
        # here we keep all vocabs by setting reduce_rate to 1.0
        wc -l $path/$outname-e$n_epoch.freq
        ./bin/unigram_aggr.out 1 $path/$outname-e$e 1.0 $min_vocab_size $vocab_size
        wc -l $path/$outname-e$n_epoch.freq
    done

    # update vocab
    ./bin/unigram_aggr.out 1 $path/$outname-e$n_epoch $reduce_rate $min_vocab_size $target_vocab_size
    init_subwords=$path/$outname-e${n_epoch}_pruned.freq
done

mv $path/*.freq $path/log 
mv $path/*.log $path/log 
mv $path/*.seg $path/log 
