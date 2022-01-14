FROM pigeatgarlic/gst-base 
WORKDIR /source

# debugging tool
RUN \
    apt install -y gdb  \
                   cmake

RUN \
    apt install -y libsoup-gnome2.4-dev \
                   libjson-glib-1.0-0 \
                   libjson-glib-dev \
                   libx11-dev \
                   xserver-xorg-dev \
                   xorg-dev



RUN git clone https://github.com/thinkonmay/personal-cloud-computing 

WORKDIR /source/personal-cloud-computing 
RUN git checkout platform-linux
RUN mkdir build 
WORKDIR /source/personal-cloud-computing/build
RUN cmake .. 
RUN make

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

CMD sleep 6000m 


