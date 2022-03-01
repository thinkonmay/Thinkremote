import { clientLog, setDebug } from "./app.js";
import { turnOffLoaddingScreen } from "./GUI.js";
import { getRTCConnection } from "./webrtc.js";

/*
* Metric serve for adaptive streaming algorithm
*/
var StreamMetric =
{
    currentTime: 0,
    AudioBitrate: 0,
    VideoBitrate: 0,
    Framerate: 0,


    PacketsLost: 0,
    AudioLatency: 0,
    VideoLatency: 0,
    TotalBandwidth:  0,
}







function  
getConnectionStats(WebRTC) 
{
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
            WebRTC.getStats(function (stats) {

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
        setDebug("unable to fetch connection stats for brower, only Chrome is supported.");
    }
}





/**
 * 
 */
export function
startCollectingStat()
{
    /**
     * statstistic control variable
     */
    var bytesReceivedStart = 0;
    var audiobytesReceivedStart = 0;
    var statsStart = new Date().getTime() / 1000;
    var statsLoop = () => {        
        getConnectionStats(getRTCConnection()).then((stats) => 
        {
            StreamMetric.AudioLatency = parseInt(stats.audioCurrentDelayMs);
            
            // Compute current video bitrate in mbps
            var now = new Date().getTime() / 1000;

            /**
             * time value of an sample
             */
            StreamMetric.currentTime = now - statsStart;



            /**
             * packets lost
             */
            StreamMetric.PacketsLost = parseInt(stats.videopacketsLost);

            /**
             * video decoder ex:ffmpeg
             */
            var connectionVideoDecoder =            stats.videocodecImplementationName;
            var connectionVideoCodecName =          stats.videoCodecName;
            var connectionStatType =                stats.videoLocalCandidateType;
            var connectionResolution =              stats.videoFrameWidthReceived + "x" + stats.videoFrameHeightReceived;

            /**
             * (volatile) framerate of the stream
             */
            StreamMetric.Framerate = parseInt(stats.videoFrameRateOutput);

            /**
             * (volatile) total bandwidth of the stream
             */
            StreamMetric.TotalBandwidth =  parseInt(stats.videoAvailableReceiveBandwidth);

            
            /**
             * (volatile) video latency
             */
            StreamMetric.VideoLatency = parseInt(stats.videoCurrentDelayMs);

                /**
                 * (volatile) video bitrate 
                 */
            StreamMetric.VideoBitrate = Math.round(parseInt((stats.videobytesReceived) - bytesReceivedStart) / (now - statsStart));
            bytesReceivedStart = parseInt(stats.videobytesReceived);


            StreamMetric.AudioBitrate = Math.round((parseInt(stats.audiobytesReceived) - audiobytesReceivedStart) / (now - statsStart));
            audiobytesReceivedStart = parseInt(stats.audiobytesReceived);



            /**
             * prepare message to report qoe metric to slave
             */         
            clientLog
            (
                {
                    FrameRate:      StreamMetric.Framerate,
    
                    AudioLatency:   StreamMetric.AudioLatency,
                    VideoLatency:   StreamMetric.VideoLatency,
    
                    AudioBitrate:   StreamMetric.AudioBitrate,
                    VideoBitrate:   StreamMetric.VideoBitrate,
    
                    TotalBandwidth: StreamMetric.TotalBandwidth,
                    PacketsLost:    StreamMetric.PacketsLost
                }
            );            

            statsStart = now;

            // Stats refresh loop.
            setTimeout(statsLoop, 1000);
        });
    };
    statsLoop();
}