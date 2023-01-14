FROM ubuntu:20.04

ARG THREADS=1

ENV CFLAGS="-fPIC"
ENV CPPFLAGS="-fPIC"
ENV CXXFLAGS="-fPIC"
ENV SOURCE_DATE_EPOCH=1397818193

ENV TZ=Europe/Amsterdam
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN echo '91.189.91.38 security.ubuntu.com >> /etc/hosts'
RUN sed -i -e 's/http:\/\/archive\.ubuntu\.com\/ubuntu\//http:\/\/mirrors\.storpool\.com\/ubuntu\/archive\//' /etc/apt/sources.list
RUN sed -i -e 's/deb /deb [trusted=yes] /' /etc/apt/sources.list
RUN apt-get update && \
    apt-get install -y \
    nano vim ccache make build-essential \
    software-properties-common automake pkg-config python \
    libtool-bin wget zip libyaml-cpp-dev \
    cpio cmake ccache build-essential git pkg-config  \
    rapidjson-dev pybind11-dev libyaml-cpp-dev python3-dev  \
    python3-virtualenv libopenblas-dev libpcre2-dev  \
    libprotobuf-dev protobuf-compiler libsqlite3-dev

RUN add-apt-repository ppa:git-core/ppa && \
    apt-get update && \
    apt-get install -y git && \
    rm -rf /var/lib/apt/lists/*

RUN git clone -b v1.2.11 --depth 1 https://github.com/madler/zlib && \
    cd zlib && \
    git reset --hard cacf7f1d4e3d44d871b605da3b647f07d718623f && \
    ./configure --static --prefix=/usr/local/zlib && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b R_2_2_9 --depth 1 https://github.com/libexpat/libexpat && \
    cd libexpat/expat && \
    git reset --hard a7bc26b69768f7fb24f0c7976fae24b157b85b13 && \
    ./buildconf.sh && \
    ./configure --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b v3.12.4 --depth 1 https://github.com/protocolbuffers/protobuf && \
    cd protobuf && \
    git reset --hard c9d2bd2fc781fe67ebf306807b9b6edb4a0d2764 && \
    ./autogen.sh && \
    ./configure --enable-static --disable-shared && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN wget https://www.openssl.org/source/openssl-1.1.1i.tar.gz && \
    echo "e8be6a35fe41d10603c3cc635e93289ed00bf34b79671a3a4de64fcee00d5242 openssl-1.1.1i.tar.gz" | sha256sum -c && \
    tar -xzf openssl-1.1.1i.tar.gz && \
    rm openssl-1.1.1i.tar.gz && \
    cd openssl-1.1.1i && \
    ./config no-shared no-dso --prefix=/usr/local/openssl && \
    make -j$THREADS && \
    make -j$THREADS install_sw && \
    rm -rf $(pwd)

RUN git clone -b v3.25.0 --depth 1 https://github.com/Kitware/CMake && \
    cd CMake && \
    git reset --hard 13e46189c7f3b39a26e9ca689bc029b7061d26a7 && \
    OPENSSL_ROOT_DIR=/usr/local/openssl ./bootstrap && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone https://github.com/kroketio/intgemm.git --recursive && \
    cd intgemm && \
    git reset --hard e70509c80883f772e01c5de1254549953dcdd6c6 && \
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DINTGEMM_DONT_BUILD_TESTS=ON . && \
    make -Cbuild -j$THREADS && \
    make -Cbuild -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone https://github.com/kroketio/sentencepiece-browsermt.git --recursive && \
    cd sentencepiece-browsermt && \
    git reset --hard d145cb7f0fb5eb68eefb79464b85f472ff94ddfc && \
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_TYPE=Release -DSPM_USE_BUILTIN_PROTOBUF=OFF -DSPM_BUILD_LIBRARY_ONLY=ON . && \
    make -Cbuild -j$THREADS && \
    make -Cbuild -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone https://github.com/kroketio/pathie-cpp.git --recursive && \
    cd pathie-cpp && \
    git reset --hard 5a9732f5a8e2c3c1508a7aef526ae4c001c4f262 && \
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Release . && \
    make -Cbuild -j$THREADS && \
    make -Cbuild -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone https://github.com/kroketio/marian-lite.git --recursive && \
    cd marian-lite && \
#    git reset --hard d060d43996929fe34de607e0b31d5e9136e161d7 && \
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_TYPE=Release -DSTATIC=ON -DSHARED=OFF && \
    make -Cbuild -j$THREADS && \
    make -Cbuild -j$THREADS install && \
    rm -rf $(pwd)

RUN git config --global --add safe.directory /kotki
