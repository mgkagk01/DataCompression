clc
clear all

fileName = 'tt_sif.y4m'; % Download it from https://media.xiph.org/video/derf/

% --- Convert the file
[mov, videoInfo] = yuv4mpeg2mov(fileName);

% --- Create an Encode and a Decoder object
encoder = Encoder(videoInfo,mov);
decoder = Decoder(videoInfo);

% --- Encode
[quantValuesFrames, motionVectors] = encoder.encode();

% --- Decode
decoder.decode(quantValuesFrames,motionVectors);