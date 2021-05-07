#!/bin/sh

tcmalloc=/usr/lib/libtcmalloc.so
jemalloc=/opt/benchmark/jemalloc/lib/libjemalloc.so
mimalloc=/opt/benchmark/mimalloc/build/libmimalloc.so
rpmalloc=/opt/benchmark/rpmalloc/bin/linux/release/x86-64/librpmallocwrap.so

executable=allocator-benchmark/build/allocator-benchmark

for name in $tcmalloc $jemalloc $mimalloc $rpmalloc; do
        echo "LD_PRELOAD=$name"
        export LD_PRELOAD=$name

        $executable 1 0 0 2 20000 50000 5000 16 1000
        $executable 2 0 0 2 20000 50000 5000 16 1000
        $executable 3 0 0 2 20000 50000 5000 16 1000
        $executable 4 0 0 2 20000 50000 5000 16 1000
        $executable 5 0 0 2 20000 50000 5000 16 1000
        $executable 6 0 0 2 20000 50000 5000 16 1000
        $executable 7 0 0 2 20000 50000 5000 16 1000
        $executable 8 0 0 2 20000 50000 5000 16 1000
        $executable 9 0 0 2 20000 50000 5000 16 1000
        $executable 10 0 0 2 20000 50000 5000 16 1000
        $executable 11 0 0 2 20000 50000 5000 16 1000
        $executable 12 0 0 2 20000 50000 5000 16 1000
        $executable 13 0 0 2 20000 50000 5000 16 1000
        $executable 14 0 0 2 20000 50000 5000 16 1000
        $executable 15 0 0 2 20000 50000 5000 16 1000
        $executable 16 0 0 2 20000 50000 5000 16 1000
        $executable 17 0 0 2 20000 50000 5000 16 1000
        $executable 18 0 0 2 20000 50000 5000 16 1000
        $executable 19 0 0 2 20000 50000 5000 16 1000
        $executable 20 0 0 2 20000 50000 5000 16 1000
        $executable 21 0 0 2 20000 50000 5000 16 1000
        $executable 22 0 0 2 20000 50000 5000 16 1000
        $executable 23 0 0 2 20000 50000 5000 16 1000
        $executable 24 0 0 2 20000 50000 5000 16 1000
done

for name in $tcmalloc $jemalloc $mimalloc $rpmalloc; do
        echo "LD_PRELOAD=$name"
        export LD_PRELOAD=$name

        $executable 1 0 1 2 20000 50000 5000 16 8000
        $executable 2 0 1 2 20000 50000 5000 16 8000
        $executable 3 0 1 2 20000 50000 5000 16 8000
        $executable 4 0 1 2 20000 50000 5000 16 8000
        $executable 5 0 1 2 20000 50000 5000 16 8000
        $executable 6 0 1 2 20000 50000 5000 16 8000
        $executable 7 0 1 2 20000 50000 5000 16 8000
        $executable 8 0 1 2 20000 50000 5000 16 8000
        $executable 9 0 1 2 20000 50000 5000 16 8000
        $executable 10 0 1 2 20000 50000 5000 16 8000
        $executable 11 0 1 2 20000 50000 5000 16 8000
        $executable 12 0 1 2 20000 50000 5000 16 8000
        $executable 13 0 1 2 20000 50000 5000 16 8000
        $executable 14 0 1 2 20000 50000 5000 16 8000
        $executable 15 0 1 2 20000 50000 5000 16 8000
        $executable 16 0 1 2 20000 50000 5000 16 8000
        $executable 17 0 1 2 20000 50000 5000 16 8000
        $executable 18 0 1 2 20000 50000 5000 16 8000
        $executable 19 0 1 2 20000 50000 5000 16 8000
        $executable 20 0 1 2 20000 50000 5000 16 8000
        $executable 21 0 1 2 20000 50000 5000 16 8000
        $executable 22 0 1 2 20000 50000 5000 16 8000
        $executable 23 0 1 2 20000 50000 5000 16 8000
        $executable 24 0 1 2 20000 50000 5000 16 8000
done

for name in $tcmalloc $jemalloc $mimalloc $rpmalloc; do
        echo "LD_PRELOAD=$name"
        export LD_PRELOAD=$name

        $executable 1 0 1 2 10000 50000 5000 16 16000
        $executable 2 0 1 2 10000 50000 5000 16 16000
        $executable 3 0 1 2 10000 50000 5000 16 16000
        $executable 4 0 1 2 10000 50000 5000 16 16000
        $executable 5 0 1 2 10000 50000 5000 16 16000
        $executable 6 0 1 2 10000 50000 5000 16 16000
        $executable 7 0 1 2 10000 50000 5000 16 16000
        $executable 8 0 1 2 10000 50000 5000 16 16000
        $executable 9 0 1 2 10000 50000 5000 16 16000
        $executable 10 0 1 2 10000 50000 5000 16 16000
        $executable 11 0 1 2 10000 50000 5000 16 16000
        $executable 12 0 1 2 10000 50000 5000 16 16000
        $executable 13 0 1 2 10000 50000 5000 16 16000
        $executable 14 0 1 2 10000 50000 5000 16 16000
        $executable 15 0 1 2 10000 50000 5000 16 16000
        $executable 16 0 1 2 10000 50000 5000 16 16000
        $executable 17 0 1 2 10000 50000 5000 16 16000
        $executable 18 0 1 2 10000 50000 5000 16 16000
        $executable 19 0 1 2 10000 50000 5000 16 16000
        $executable 20 0 1 2 10000 50000 5000 16 16000
        $executable 21 0 1 2 10000 50000 5000 16 16000
        $executable 22 0 1 2 10000 50000 5000 16 16000
        $executable 23 0 1 2 10000 50000 5000 16 16000
        $executable 24 0 1 2 10000 50000 5000 16 16000
done

for name in $tcmalloc $jemalloc $mimalloc $rpmalloc; do
        echo "LD_PRELOAD=$name"
        export LD_PRELOAD=$name

        $executable 1 0 2 2 10000 30000 3000 128 64000
        $executable 2 0 2 2 10000 30000 3000 128 64000
        $executable 3 0 2 2 10000 30000 3000 128 64000
        $executable 4 0 2 2 10000 30000 3000 128 64000
        $executable 5 0 2 2 10000 30000 3000 128 64000
        $executable 6 0 2 2 10000 30000 3000 128 64000
        $executable 7 0 2 2 10000 30000 3000 128 64000
        $executable 8 0 2 2 10000 30000 3000 128 64000
        $executable 9 0 2 2 10000 30000 3000 128 64000
        $executable 10 0 2 2 10000 30000 3000 128 64000
        $executable 11 0 2 2 10000 30000 3000 128 64000
        $executable 12 0 2 2 10000 30000 3000 128 64000
        $executable 13 0 2 2 10000 30000 3000 128 64000
        $executable 14 0 2 2 10000 30000 3000 128 64000
        $executable 15 0 2 2 10000 30000 3000 128 64000
        $executable 16 0 2 2 10000 30000 3000 128 64000
        $executable 17 0 2 2 10000 30000 3000 128 64000
        $executable 18 0 2 2 10000 30000 3000 128 64000
        $executable 19 0 2 2 10000 30000 3000 128 64000
        $executable 20 0 2 2 10000 30000 3000 128 64000
        $executable 21 0 2 2 10000 30000 3000 128 64000
        $executable 22 0 2 2 10000 30000 3000 128 64000
        $executable 23 0 2 2 10000 30000 3000 128 64000
        $executable 24 0 2 2 10000 30000 3000 128 64000
done

for name in $tcmalloc $jemalloc $mimalloc $rpmalloc; do
        echo "LD_PRELOAD=$name"
        export LD_PRELOAD=$name

        $executable 1 0 2 2 10000 20000 2000 512 160000
        $executable 2 0 2 2 10000 20000 2000 512 160000
        $executable 3 0 2 2 10000 20000 2000 512 160000
        $executable 4 0 2 2 10000 20000 2000 512 160000
        $executable 5 0 2 2 10000 20000 2000 512 160000
        $executable 6 0 2 2 10000 20000 2000 512 160000
        $executable 7 0 2 2 10000 20000 2000 512 160000
        $executable 8 0 2 2 10000 20000 2000 512 160000
        $executable 9 0 2 2 10000 20000 2000 512 160000
        $executable 10 0 2 2 10000 20000 2000 512 160000
        $executable 11 0 2 2 10000 20000 2000 512 160000
        $executable 12 0 2 2 10000 20000 2000 512 160000
        $executable 13 0 2 2 10000 20000 2000 512 160000
        $executable 14 0 2 2 10000 20000 2000 512 160000
        $executable 15 0 2 2 10000 20000 2000 512 160000
        $executable 16 0 2 2 10000 20000 2000 512 160000
        $executable 17 0 2 2 10000 20000 2000 512 160000
        $executable 18 0 2 2 10000 20000 2000 512 160000
        $executable 19 0 2 2 10000 20000 2000 512 160000
        $executable 20 0 2 2 10000 20000 2000 512 160000
        $executable 21 0 2 2 10000 20000 2000 512 160000
        $executable 22 0 2 2 10000 20000 2000 512 160000
        $executable 23 0 2 2 10000 20000 2000 512 160000
        $executable 24 0 2 2 10000 20000 2000 512 160000
done
