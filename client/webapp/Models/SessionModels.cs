using System.Collections.Generic;

namespace remote.Models
{
    public class SessionClient
    {
        public string signallingurl { get; set; }

        public string turnip { get; set; }

        public string turnuser { get; set; }

        public string turnpassword{ get; set; }

        public string turn{ get; set; }

        public List<string> stuns { get; set; }

        public Codec audiocodec { get; set; }

        public Codec videocodec { get; set; }
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
}