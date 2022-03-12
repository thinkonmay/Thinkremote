
var Module = 
{
    "CORE_MODULE":          0,
    "CLIENT_MODULE":        1,
    "LOADER_MODULE":        2,
    "AGENT_MODULE":         3,
    "HOST_MODULE":          4
};


var HidOpcode = 
{
    "KEYUP":                100,
    "KEYDOWN":              101,

    "MOUSE_WHEEL":          102,
    "MOUSE_MOVE":           103,
    "MOUSE_UP":             104,
    "MOUSE_DOWN":           105,
};

var QoEMode = 
{
    "ULTRA_LOW_CONST":                     1,
    "LOW_CONST":                           2,
    "MEDIUM_CONST":                        3,
    "HIGH_CONST":                          4,
    "VERY_HIGH_CONST":                     5,
    "ULTRA_HIGH_CONST":                    6,
    "SEGMENTED_ADAPTIVE_BITRATE":          7,
    "NON_OVER_SAMPLING_ADAPTIVE_BITRATE":  8,
    "OVER_SAMPLING_ADAPTIVE_BITRATE":      9,
    "PREDICTIVE_ADAPTIVE_BITRATE":         10
};

var Codec = 
{
    "CODEC_H265":           0,
    "CODEC_H264":           1,
    "CODEC_VP9":            2,
    "OPUS_ENC":             3,
    "AAC_ENC":              4
};

var CoreEngine = 
{
    GSTREAMER: 0,
    CHROME:    1
};

var DeviceType = 
{
    WEBAPP: 0
};

