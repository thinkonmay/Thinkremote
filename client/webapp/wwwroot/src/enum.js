
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
    "KEYUP":                0,
    "KEYDOWN":              1,
    "MOUSE_WHEEL":          2,
    "MOUSE_MOVE":           3,
    "MOUSE_UP":             4,
    "MOUSE_DOWN":           5,
    "POINTER_LOCK":         6,
    "KEYRESET":             7
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