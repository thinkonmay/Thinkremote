Personal cloud computing
===================================

Personal cloud computing is the technology stack allow you to access other computer remotely which

- Based on WebRTC video streaming technology and Gstreamer multimedia framework for video and audio pipeline handling
- Is capable of H265, H264 video streaming, OPUS audio codec audio streaming 
- Is capable of hardware accelerated video encoding based on window's d3d11 video encoding core 



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
Switch to development environment by set
- `development.h: DEVELOPMENT_ENVIRONMENT to TRUE`
* Run client and worker build by run corresponding automated build bat file
  * `cd worker`
  * `./build.bat`
  * `cd ../client`
  * `./build.bat`
* Host signalling test server
  * `cd test/Signalling`
  * `dotnet run .`
* `worker/bin/session-core.exe`
* `client/bin/remote-app.exe`


Join us
-----------
- Website: https://www.thinkmay.net
- Slack (chat channel): https://join.slack.com/t/thinkmayworkspace/shared_invite/zt-ywglslgj-fQb4Po4JagVaHbZ8wwiqpg
- Architecture document (miro): https://miro.com/app/board/o9J_lTKComc=/?invite_link_id=202014558866
- Detailed document (notion) https://thinkonmay.notion.site/5a4909c660374a4ca0286d766bf3b9f1?v=bd0da1b672c14c6fbe2f2ad4d29b99b7

How to use
-----------
Personal cloud computing github repository is managed by Thinkmay, we also provide the infrastructure you needed to use personal cloud computing  

- Request a demo by email us: contact@thinkmay.net

Licensing
-----------
Thinkmay streaming is distributed uner GNU General Public License v3

Contributing:
-----------
- If you want to contribute to this repository email us at contact@thinkmay.net
