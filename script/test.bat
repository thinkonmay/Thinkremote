gst-launch-1.0 d3d11screencapturesrc ! queue ! d3d11convert ! queue ! d3d11download ! queue ! mfh264enc ! queue ! rtph264pay ! queue ! rtph264depay ! queue ! h264parse ! queue ! d3d11h264dec ! queue ! d3d11videosink