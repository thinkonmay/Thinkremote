namespace Signalling.Models
{
    public class WebSocketMessage
    {
        public string RequestType { get; set; }
        public string Content { get; set; }
    }

    public class WebSocketMessageResult
    {
        public const string OFFER_SDP =  "OFFER_SDP";
        public const string OFFER_ICE =  "OFFER_ICE";
        public const string REQUEST_STREAM = "REQUEST_STREAM";
    }

    public class StreamConfig
    {
        public int screen_height {get;set;}

        public int screen_width {get;set;}

        public QoEMode mode {get;set;}

        public Codec codec_audio {get;set;}

        public Codec codec_video {get;set;}
    }

    public enum Codec
    {
        CODEC_H265,
        CODEC_H264,
        CODEC_VP8,
        CODEC_VP9,

        OPUS_ENC,
        AAC_ENC
    }

    public enum QoEMode
    {
        ULTRA_LOW_CONST = 1,
        LOW_CONST,
        MEDIUM_CONST,
        HIGH_CONST,
        VERY_HIGH_CONST,
        ULTRA_HIGH_CONST,
    }
}
