# Stroke to fill conversion

This is the source code and test-harness for the paper

> Diego Nehab, 2020. Converting stroked primitives to filled primitives,
> *ACM Transaction on Graphics (Proceedings of ACM SIGGRAPH 2020)*, 39(4)

It was updated to work on the newest versions of Unbuntu and macOS after the
paper was published.

Visit the [project page](http://www.impa.br/~diego/projects/Neh20) for more details.

## Installation of general dependencies

On Ubuntu 20.04

```
    sudo apt-get install build-essential git subversion wget libb64-dev libboost-dev libcairo2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libpng-dev liblapacke-dev libreadline-dev zlib1g-dev libegl1-mesa-dev libglew-dev libgsl-dev libwebp-dev clang-9 libjpeg-turbo8-dev qt5-default maven openjdk-8-jre openjdk-11-jre

    sudo mkdir -p /usr/local/lib/pkgconfig
    sudo cp b64.pc /usr/local/lib/pkgconfig/

    wget http://www.lua.org/ftp/lua-5.3.5.tar.gz
    tar -zxvf lua-5.3.5.tar.gz
    cd lua-5.3.5
    patch -p1 < ../luapp.patch
    make CC=g++ MYCFLAGS="-x c++ -fopenmp" MYLIBS="-lgomp" linux
    sudo make install
    sudo ln -s /usr/local/bin/luapp5.3 /usr/local/bin/luapp
    sudo cp ../luapp53.pc /usr/local/lib/pkgconfig/
    cd ..
    \rm -rf lua-5.3.5

```

On macOS Catalina using macports


```
    sudo port install clang-9.0 git subversion wget boost pkgconfig libb64 libpng qt5 cairo freetype harfbuzz-icu gsl mesa

    sudo mkdir -p /usr/local/lib/pkgconfig
    sudo cp b64.pc /usr/local/lib/pkgconfig/

    wget http://www.lua.org/ftp/lua-5.3.5.tar.gz
    tar -zxvf lua-5.3.5.tar.gz
    cd lua-5.3.5
    patch -p1 < ../luapp.patch
    make CC=clang++-mp-9.0 MYCFLAGS="-x c++ -fopenmp" MYLIBS="-L/opt/local/lib/libomp -lgomp" macosx
    sudo make install
    sudo ln -s /usr/local/bin/luapp5.3 /usr/local/bin/luapp
    sudo mkdir -p /usr/local/lib/pkgconfig
    sudo cp ../luapp53.pc /usr/local/lib/pkgconfig/
    cd ..
    \rm -rf lua-5.3.5
```

## Installation of third-party stroker dependencies

Go through each directory in

```
    strokers/agg
    strokers/cairo
    strokers/gs
    strokers/livarot
    strokers/mupdf
    strokers/openvg-ri
    strokers/skia
```

and read the README file for instructions to build the dependencies.

> **WARNING: Please be careful: don't blindly execute the commands.
> These commands worked for us but are only meant as an illustration.**

Now you can build the entire framework

```
    export vg_build_strokers=yes
    export vg_build_distroke=yes
    export vg_build_skia=yes
    # Optionally, enable other drivers
    # export vg_build_qt5=yes
    # export vg_build_cairo=yes
    # export vg_build_nvpr=yes
    make
```

The main command for the stroke tests is test-strokers.lua.
Run

```
    luapp test-strokers.lua -help
```

for the options.

There are also a few scripts inside the strokers/ directory.
We used them to create the animations in the supplemental meterials.
