#!/bin/bash

# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -ex

function finish {
    rm -f /var/run/appconfig/xserver_ready
}
trap finish EXIT

# Symlink for X11 virtual terminal
ln -sf /dev/ptmx /dev/tty7

# Forward mouse input control socket to shared pod volume.
if [[ -S /tmp/.uinput/mouse0ctl ]]; then
    echo "Forwarding socket /tmp/.uinput/mouse0ctl to /var/run/appconfig/mouse0ctl"
    nohup socat UNIX-RECV:/var/run/appconfig/mouse0ctl,reuseaddr UNIX-CLIENT:/tmp/.uinput/mouse0ctl &
fi

# Start dbus
rm -rf /var/run/dbus
dbus-uuidgen | tee /var/lib/dbus/machine-id
mkdir -p /var/run/dbus
dbus-daemon --config-file=/usr/share/dbus-1/system.conf --print-address

export RESOLUTION=${RESOLUTION:-"1920x1080"}
export PID=$$
echo "Starting X11 server with software video driver."
Xvfb -screen ${DISPLAY} ${RESOLUTION}x24 +extension RANDR +extension GLX +extension MIT-SHM -nolisten tcp -noreset -shmem &
PID=$!

# Wait for X11 to start
set +x
echo "Waiting for X socket"
until [[ -S /tmp/.X11-unix/X${DISPLAY/:/} ]]; do sleep 1; done
echo "X socket is ready"
set -x

echo "Waiting for X11 startup"
until xhost + >/dev/null 2>&1; do sleep 1; done
echo "X11 startup complete"

echo "INFO: Setting mode to: ${RESOLUTION}"
xrandr -s "${RESOLUTION}"

# Notify sidecar containers
if [[ ${DISPLAY} == ":0" ]]; then
    touch /var/run/appconfig/xserver_ready
else
    touch /var/run/appconfig/xserver_${DISPLAY}_ready
fi

# Wait for background process
wait $PID