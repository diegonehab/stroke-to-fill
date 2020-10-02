FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN \
    apt-get update && \
    apt-get install --no-install-recommends -y \
        ca-certificates build-essential git wget \
        libb64-dev libboost-dev libcairo2-dev libfreetype6-dev \
        libharfbuzz-dev libicu-dev libpng-dev liblapacke-dev \
        libreadline-dev zlib1g-dev && \
    rm -rf /var/lib/apt/lists/*

RUN \
    git clone --branch feature/reproduce https://github.com/diegonehab/stroke-to-fill.git && \
    cd stroke-to-fill && \
    wget http://www.lua.org/ftp/lua-5.3.5.tar.gz && \
    tar -zxvf lua-5.3.5.tar.gz && \
    cd lua-5.3.5 && \
    patch -p1 < ../luapp.patch && \
    make -j$(nproc) CC=g++ MYCFLAGS="-x c++ -fopenmp" MYLIBS="-lgomp" linux && \
    make install && \
    cd .. && \
    \rm -rf lua-5.3.5 && \
    mkdir -p /usr/local/lib/pkgconfig && \
    ln -s /usr/local/bin/luapp5.3 /usr/local/bin/luapp && \
    cp luapp53.pc /usr/local/lib/pkgconfig/ && \
    cp b64.pc /usr/local/lib/pkgconfig/ && \
    make -j$(nproc)

WORKDIR /stroke-to-fill

CMD ["/bin/bash", "-l"]
