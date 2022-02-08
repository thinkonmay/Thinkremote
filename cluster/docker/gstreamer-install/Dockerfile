FROM ubuntu:20.04 AS base

# Install essentials
RUN \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        curl \
        build-essential \
        ca-certificates \
        git \
        vim

RUN apt-get -y install libgstreamer1.0-dev  \
                        libgstreamer-plugins-base1.0-dev  \
                        libgstreamer-plugins-bad1.0-dev  \
                        gstreamer1.0-plugins-base  \
                        gstreamer1.0-plugins-good  \
                        gstreamer1.0-plugins-bad  \
                        gstreamer1.0-plugins-ugly  \
                        gstreamer1.0-libav  \
                        gstreamer1.0-doc  \
                        gstreamer1.0-tools  \
                        gstreamer1.0-x  \
                        gstreamer1.0-alsa  \
                        gstreamer1.0-gl  \
                        gstreamer1.0-gtk3  \
                        gstreamer1.0-qt5  \
                        gstreamer1.0-pulseaudio 

WORKDIR /opt
