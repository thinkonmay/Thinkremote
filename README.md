Thinkremote
===================================
Thinkremote is the technology stack allow you to access your computer remotely

**Demo**

https://user-images.githubusercontent.com/64737125/146868760-6cb69504-ddca-4660-8be1-8d568b16f443.mp4


Features
--------------
- Based on WebRTC video streaming technology and Gstreamer multimedia framework for video and audio pipeline handling
- Is capable of H265, H264 video streaming, OPUS audio codec audio streaming 
- Is capable of hardware accelerated video encoding based on window's d3d11 video encoding core 

Join us
-----------
[Website](https://www.thinkmay.net) |
[Chat](https://join.slack.com/t/thinkmayworkspace/shared_invite/zt-ywglslgj-fQb4Po4JagVaHbZ8wwiqpg) |
[Architecture](https://miro.com/app/board/o9J_lTKComc=/?invite_link_id=202014558866) |
[Document](https://thinkonmay.notion.site/5a4909c660374a4ca0286d766bf3b9f1?v=bd0da1b672c14c6fbe2f2ad4d29b99b7)

Requirement to build
-------------------------
  - [Gstreamer 1.19.2](https://gstreamer.freedesktop.org/) - A complete, cross-platform solution to record, convert and stream audio and video.
    1. [Download Gstreamer package](https://gstreamer.freedesktop.org/data/pkg/windows/1.19.2/msvc/gstreamer-1.0-devel-msvc-x86_64-1.19.2.msi) 
    1. [Download Gstreamer development package](https://gstreamer.freedesktop.org/data/pkg/windows/1.19.2/msvc/gstreamer-1.0-msvc-x86_64-1.19.2.msi) 
    2. Once the installer is downloaded, install gstreamer to C:/ Drive.
  - [VB Cable](https://vb-audio.com/Cable) - Create a fake audio device for audio capturing.  
    1. [Download VB Cable driver](https://download.vb-audio.com/Download_CABLE/VBCABLE_Driver_Pack43.zip) 
    2. Restart your computer after install VC Cable driver, your VB Input audio enndpoint should be enabled
  - [Visual Studio](https://visualstudio.microsoft.com/) - Build tool on window (2022 version is recommended).  
    1. [Download Visual Studio from Microsoft](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&rel=17#install) 
    2. If your downloaded version is not preview/2022, please go to build.bat and change the path to vcvarsall.bat.

Client module
----------------
Client module is organize by platforms, each platform has their own techstack


* Window native application dashboard is run on electron, currently we have two core engine: 
  * one based on Gstreamer multimedia framework 
  * one based on core implementation of video decode on electron 
  
(Note: only Window Gstreamer core engine implementation is capable of H265 video streaming)

* Progressive webapp hosted by Thinkmay server and can be run from any browser, however, the video and audio quality of chromium default decoder is much behind native application and is not recommended by us 




Worker module
--------------
Worker module is the agent installed on worker node in order to allow it publicly accessible by user. It has three submodule with different role
* Agent submodule for session initialization
* Session core implement WebRTC
* Cluster module for managing worker cluster


Worker module is a window application written in C and based on Gstreamer multimedia framework



Test instruction
---------------
* Switch to development environment by set
  * `development.h: DEVELOPMENT_ENVIRONMENT to TRUE`
* Run client and worker build by run corresponding automated build bat file
  * `./build.bat`
* Host signalling test server
  * `cd test/Signalling`
  * `dotnet run .`
* Run session-core and remote-app binary
  * `worker/bin/session-core.exe`
  * `client/bin/remote-app.exe`



How to use
-----------
Thinkremote github repository is managed by Thinkmay, we also provide the infrastructure you needed to use personal cloud computing  

- Request a demo by email us: contact@thinkmay.net

Licensing
-----------
Thinkremote is distributed uner GNU General Public License v3

Contributing
-----------
- If you want to contribute to this repository email us at contact@thinkmay.net
