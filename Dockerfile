FROM ubuntu:jammy

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get -y install wget xz-utils libxml2-dev cmake build-essential libboost-system-dev libboost-filesystem-dev libcrypto++-dev libgtkmm-3.0-dev g++-9 liblzma-dev && \
    mkdir /libxml && \
    wget http://ftp.gnome.org/pub/GNOME/sources/libxml++/3.0/libxml++-3.0.1.tar.xz -O /libxml/libxml++-3.0.1.tar.xz && \
    cd /libxml && \
    tar xf /libxml/libxml++-3.0.1.tar.xz --strip-components=1 && \
    ./configure --enable-static && \
    make && \
    make install

WORKDIR /project