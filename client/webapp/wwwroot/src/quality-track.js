var applyTimestamp = (msg) => {
    var now = new Date();
    var ts = now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds();
    return "[" + ts + "]" + " " + msg;
}


function  
getConnectionStats() 
{
    var pc = app.Webrtc;

    var connectionDetails = {};   // the final result object.

    if (window.chrome) {  // checking if chrome

        var reqFields = [
            'googLocalCandidateType',
            'googRemoteCandidateType',
            'packetsReceived',
            'packetsLost',
            'bytesReceived',
            'googFrameRateReceived',
            'googFrameRateOutput',
            'googCurrentDelayMs',
            'googFrameHeightReceived',
            'googFrameWidthReceived',
            'codecImplementationName',
            'googCodecName',
            'googAvailableReceiveBandwidth'
        ];

        return new Promise(function (resolve, reject) {
            pc.getStats(function (stats) {

                var filteredVideo = stats.result().filter(function (e) 
                {
                    if ((e.id.indexOf('Conn-video') === 0 && e.stat('googActiveConnection') === 'true') ||
                        (e.id.indexOf('ssrc_') === 0 && e.stat('mediaType') === 'video') ||
                        (e.id == 'bweforvideo')) return true;
                });



                if (!filteredVideo) {return reject('Something is wrong...');}
                filteredVideo.forEach((f) => {
                    reqFields.forEach((e) => {
                        var statValue = f.stat(e);
                        if (statValue != "") {
                            connectionDetails['video' + e.replace('goog', '')] = statValue;
                        }
                    });
                });

                var filteredAudio = stats.result().filter(function (e) 
                {
                    if ((e.id.indexOf('Conn-audio') === 0 && e.stat('googActiveConnection') === 'true') ||
                        (e.id.indexOf('ssrc_') === 0 && e.stat('mediaType') === 'audio') ||
                        (e.id == 'bweforaudio')) return true;
                });


                if (!filteredAudio) return reject('Something is wrong...');
                filteredAudio.forEach((f) => {
                    reqFields.forEach((e) => {
                        var statValue = f.stat(e);
                        if (statValue != "") {
                            connectionDetails['audio' + e.replace('goog', '')] = statValue;
                        }
                    });
                });
                resolve(connectionDetails);
            });
        });

    } else {
        app.setError("unable to fetch connection stats for brower, only Chrome is supported.");
    }
}


/**
 * Captures display and video dimensions required for computing mouse pointer position.
 * app should be fired whenever the window size changes.
 */
function
windowCalculate() 
{
    /**
     * size of video element (included its border) on client screen
     * (displayed video size)
     */
    app.Screen.clientWidth = app.VideoElement.offsetWidth;
    app.Screen.clientHeight = app.VideoElement.offsetHeight;

    /**
     * actual video width and height of incoming stream 
     * (registered in session initialize step)
     */
    app.Screen.StreamWidth =  app.VideoElement.videoWidth;
    app.Screen.StreamHeight = app.VideoElement.videoHeight;

    /**
     * fraction between displayed video size and incoming stream framesize
     * (both width and height fraction is acceptable)
     */
    app.Screen.fraction = Math.min
        (app.Screen.clientWidth  / app.Screen.StreamWidth, 
         app.Screen.clientHeight / app.Screen.StreamHeight);



    const vpWidth =  app.Screen.StreamWidth  * app.Screen.fraction;
    const vpHeight = app.Screen.StreamHeight * app.Screen.fraction;



    /**
     * reposition mouse after screen resolution has been changed
     */
    app.Mouse = {
        /**
         * relation between frame size and actual window size
         */
        "mouseMultiX": app.Screen.StreamWidth / vpWidth,
        "mouseMultiY": app.Screen.StreamHeight / vpHeight,

        /**
         * 
         */
        "mouseOffsetX": Math.max((app.Screen.clientWidth  - vpWidth) / 2.0, 0),
        "mouseOffsetY": Math.max((app.Screen.clientHeight - vpHeight) / 2.0, 0),


        /**
         * 
         */
        "centerOffsetX": (document.documentElement.clientWidth - app.VideoElement.offsetWidth) / 2.0,
        "centerOffsetY": (document.documentElement.clientHeight - app.VideoElement.offsetHeight) / 2.0,

        /**
         * 
         */
        "scrollX": window.scrollX,
        "scrollY": window.scrollY,

        /**
         * 
         */
        "frameW": app.Screen.StreamWidth,
        "frameH": app.Screen.StreamHeight,
    };
    
    /**
     * resize slave window if client window has been resize
     */
    if(app.adaptiveScreenSize)
    {
        app.QoeReset();
    }
}



/**
 * get window resolution
 * @returns 2 element list control screen width and height
 */
function 
getWindowResolution() 
{
    return [ /**/
        parseInt(app.VideoElement.offsetWidth * window.devicePixelRatio),
        parseInt(app.VideoElement.offsetHeight * window.devicePixelRatio)
    ];
}




function 
ResizeWindow()
{
    app.windowResolution = getWindowResolution();
    app.logEntries.push(`Window size changed: ${app.windowResolution[0]}x${app.windowResolution[1]}`);
}



function
get_stats()
{
    /**
     * statstistic control variable
     */
    var bytesReceivedStart = 0;
    var audiobytesReceivedStart = 0;
    var statsStart = new Date().getTime() / 1000;
    var statsLoop = () => {        
        getConnectionStats().then((stats) => 
        {
            if (app.audioEnabled) {
                app.adaptive.AudioLatency = parseInt(stats.audioCurrentDelayMs);


            } else {
                stats.audiobytesReceived = 0;
            }
            
            // Compute current video bitrate in mbps
            var now = new Date().getTime() / 1000;
            /**
             * time value of an sample
             */
            app.adaptive.currentTime = now - statsStart;


            app.connectionStatType = stats.videoLocalCandidateType;

            /**
             * packets lost
             */
            app.adaptive.PacketsLost = parseInt(stats.videopacketsLost);

            /**
             * video codec,ex HEVC
             */
            app.connectionVideoCodecName = stats.videoCodecName;
            /**
             * video decoder ex:ffmpeg
             */
            app.connectionVideoDecoder = stats.videocodecImplementationName;

            app.connectionResolution = stats.videoFrameWidthReceived + "x" + stats.videoFrameHeightReceived;

            /**
             * (volatile) framerate of the stream
             */
            app.adaptive.Framerate = parseInt(stats.videoFrameRateOutput);

            /**
             * (volatile) total bandwidth of the stream
             */
            app.adaptive.TotalBandwidth =  parseInt(stats.videoAvailableReceiveBandwidth);

            
            /**
             * (volatile) video latency
             */
            app.adaptive.VideoLatency = parseInt(stats.videoCurrentDelayMs);

                /**
                 * (volatile) video bitrate 
                 */
            app.adaptive.VideoBitrate = Math.round(parseInt((stats.videobytesReceived) - bytesReceivedStart) / (now - statsStart));
            bytesReceivedStart = parseInt(stats.videobytesReceived);


            app.adaptive.AudioBitrate = Math.round((parseInt(stats.audiobytesReceived) - audiobytesReceivedStart) / (now - statsStart));
            audiobytesReceivedStart = parseInt(stats.audiobytesReceived);


            /**
             * prepare message to report qoe metric to slave
             */         
            sendControlDC
            (
                JSON.stringify(
                {
                    "FrameRate": app.adaptive.Framerate,
    
                    "AudioLatency": app.adaptive.AudioLatency,
                    "VideoLatency": app.adaptive.VideoLatency,
    
                    "AudioBitrate": app.adaptive.AudioBitrate,
                    "VideoBitrate": app.adaptive.VideoBitrate,
    
                    "TotalBandwidth": app.adaptive.TotalBandwidth,
                    "PacketsLost": app.adaptive.PacketsLost
                })
            );            
            statsStart = now;

            // Stats refresh loop.
            setTimeout(statsLoop, 1000);
        });
    };
    statsLoop();
}