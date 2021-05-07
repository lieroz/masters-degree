FROM archlinux:latest

RUN pacman --noconfirm -Syu

# setup allocator libraries
RUN pacman --noconfirm -S \
    git \
    gcc \
    make \
    python \
    bazel \
    autoconf \
    cmake \
    ninja

WORKDIR opt/benchmark

RUN git clone https://github.com/google/tcmalloc.git && \
    cd tcmalloc && bazel build //tcmalloc:tcmalloc

RUN git clone https://github.com/jemalloc/jemalloc.git && \
    cd jemalloc && git checkout 5.2.1 && ./autogen.sh && ./configure && make -j

RUN git clone https://github.com/microsoft/mimalloc.git && \
    cd mimalloc && git checkout v2.0.1 && mkdir build && cd build && cmake .. && make -j

RUN git clone https://github.com/mjansson/rpmalloc.git && \
    cd rpmalloc && git checkout 1.4.2 && python3 configure.py --toolchain gcc && ninja
