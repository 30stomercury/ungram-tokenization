set -ex

infilepath=$1 # input file
path=$2 # output path
target_vocab_size=$3
min_vocab_size=$4 # canonical set of vocabs
init_subwords=$5 # init substrings: xxx.freq
n_epoch=4
n_split=30
reduce_rate=0.7
name=test
vocab_size=$(wc -l < $init_subwords)
outname=$name-viterbi-$vocab_size
infilename=$(basename $infilename)


# split
mkdir -p $path/log
python3 tools/split.py $infilepath $n_split
mkdir -p splits
mv $infilepath.* splits

while [ "$vocab_size" -gt $target_vocab_size ]
do
    echo "The current vocab size $vocab_size, init with $init_subwords, the target vocab size is $target_vocab_size"
    # EM
    vocab_size=$(wc -l < $init_subwords)
    outname=$name-viterbi-$vocab_size
    cp $init_subwords $path/$outname-e0.freq
    for e in `seq 1 $n_epoch`; do
        # E step
        for i in `seq 1 $n_split`; do
            echo "./bin/unigram_decode.out splits/$infilename.$i $path/$outname-e$e.$i $vocab_size $path/$outname-e$((e-1))"
        done > $path/myjobs-$vocab_size
        sbatch -J unigram-$vocab_size --output=slurm/%A_%a.out --wait --array=1-$n_split \
               --cpus-per-task 3 tools/run-array.sh $path/myjobs-$vocab_size

        # keep log
        if [[ "$vocab_size" != "$target_vocab_size" || "$e" != "$n_epoch" ]]; then
            for i in `seq 1 $n_split`; do
                rm $path/$outname-e$e.$i
                rm $path/$outname-e$e.$i.seg
            done
        fi

        # M step
        # ./bin/unigram_aggr.out n_split output reduce_rate min_vocab_size current_vocab_size
        # here we keep all vocabs by setting reduce_rate to 1.0
        ./bin/unigram_aggr.out $n_split $path/$outname-e$e 1.0 $min_vocab_size $vocab_size
    done

    # update vocab
    ./bin/unigram_aggr.out $n_split $path/$outname-e$n_epoch $reduce_rate $min_vocab_size $target_vocab_size
    init_subwords=$path/$outname-e${n_epoch}_pruned.freq
done

# aggregatiom
mv $path/*.freq $path/log 
mv $path/*.log $path/log 
mv $path/*.seg $path/log 
awk 1 $path/$outname-e$n_epoch.* > $path/$outname-e$n_epoch
for i in `seq 1 $n_split`; do
    rm $path/$outname-e$e.$i
done
