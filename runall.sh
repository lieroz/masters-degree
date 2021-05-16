#!/bin/sh

tcmalloc=/usr/lib/libtcmalloc.so
jemalloc=/opt/benchmark/jemalloc/lib/libjemalloc.so
mimalloc=/opt/benchmark/mimalloc/build/libmimalloc.so
rpmalloc=/opt/benchmark/rpmalloc/bin/linux/release/x86-64/librpmallocwrap.so

executable=allocator-benchmark/build/allocator-benchmark
threads_count=16

for name in '' $tcmalloc $jemalloc $mimalloc $rpmalloc; do
        echo "LD_PRELOAD=$name"
        export LD_PRELOAD=$name

        for i in $( seq 1 $threads_count); do
            $executable $i 0 0 2 20000 50000 5000 16 1000
        done
done

for name in '' $tcmalloc $jemalloc $mimalloc $rpmalloc; do
        echo "LD_PRELOAD=$name"
        export LD_PRELOAD=$name


        for i in $( seq 1 $threads_count); do
            $executable $i 0 1 2 20000 50000 5000 16 8000
        done
done

for name in '' $tcmalloc $jemalloc $mimalloc $rpmalloc; do
        echo "LD_PRELOAD=$name"
        export LD_PRELOAD=$name


        for i in $( seq 1 $threads_count); do
            $executable $i 0 1 2 10000 50000 5000 16 16000
        done
done

for name in '' $tcmalloc $jemalloc $mimalloc $rpmalloc; do
        echo "LD_PRELOAD=$name"
        export LD_PRELOAD=$name


        for i in $( seq 1 $threads_count); do
            $executable $i 0 2 2 10000 30000 3000 128 64000
        done
done

for name in '' $tcmalloc $jemalloc $mimalloc $rpmalloc; do
        echo "LD_PRELOAD=$name"
        export LD_PRELOAD=$name


        for i in $( seq 1 $threads_count); do
            $executable $i 0 2 2 10000 20000 2000 512 160000
        done
done
