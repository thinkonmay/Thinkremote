FROM ubuntu:20.04 AS base

# Install essentials
RUN \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        curl \
        build-essential \
        ca-certificates \
        git \
        vim

WORKDIR /opt


# cloner repo for each gstreamer module
RUN git clone https://github.com/Gstreamer/gstreamer.git && cd gstreamer && git checkout tags/1.19.2
RUN git clone https://github.com/Gstreamer/gst-plugins-base.git && cd gst-plugins-base && git checkout tags/1.19.2
RUN git clone https://github.com/Gstreamer/gst-plugins-good && cd gst-plugins-good && git checkout tags/1.19.2
RUN git clone https://github.com/Gstreamer/gst-plugins-bad && cd gst-plugins-bad && git checkout tags/1.19.2
RUN git clone https://github.com/Gstreamer/gst-plugins-ugly && cd gst-plugins-ugly && git checkout tags/1.19.2

WORKDIR /opt

# Install base build deps
RUN \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        autopoint \
        autoconf \
        automake \
        autotools-dev \
        libtool \
        gettext \
        bison \
        flex \
        gtk-doc-tools \
        libtool-bin \
        libgtk2.0-dev \
        libgl1-mesa-dev \
        libopus-dev \
        libpulse-dev \
        libgirepository1.0-dev

# Install meson build deps
RUN \
    apt-get update && apt install -y python3-pip python-gi-dev ninja-build && \
    pip3 install meson

# Build gstreamer
RUN \
    cd /opt/gstreamer && \
    meson build --prefix=/usr && \
    ninja -C build install

# Build gstreamer-base
RUN \
    cd /opt/gst-plugins-base && \
    meson build --prefix=/usr && \
    ninja -C build install

# Install deps for gst-plugins-good
RUN \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        libvpx-dev \
        libvpx6

# Build gst-plugins-good
RUN \
    cd /opt/gst-plugins-good && \
    meson build --prefix=/usr && \
    ninja -C build install

# Install deps for gst-plugins-bad
RUN \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        libwebrtc-audio-processing-dev \
        libssl-dev \
        libsrtp2-dev

# Install usrsctp from source
ARG USRSCTP_VERSION=0.9.5.0
RUN \
    git clone https://github.com/sctplab/usrsctp.git && \
    cd usrsctp && git checkout tags/${USRSCTP_VERSION} && \
    ./bootstrap && ./configure --prefix=/usr && \
        make && make install && make clean

# Install libnice from source
ARG LIBNICE_VERSION=0.1.18
RUN \
    git clone https://gitlab.freedesktop.org/libnice/libnice.git && \
    cd libnice && git checkout tags/${LIBNICE_VERSION} && \
    meson build --prefix=/usr && \
    ninja -C build install

# Install gst-plugins-bad from source
RUN \
    cd /opt/gst-plugins-bad && \
    meson build --prefix=/usr && \
    ninja -C build install

# Build and install gst-plugins-ugly from source
# This package includes the x264 encoder for non-nvenc accelerated pipelines.
RUN \
    apt-get update && apt install -y libx264-155 libx264-dev

RUN \
    cd /opt/gst-plugins-ugly && \
    meson build --prefix=/usr && \
    ninja -C build install