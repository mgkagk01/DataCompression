clc 
clear all
% === Implementation of Daubechies 9/7 wavelet

% --- Load Lenna
lena = imread('Lenna.png');
lena = double(rgb2gray(lena)) - 128;


% --- Define Daubechies 9/7 filters
lowAna = [0.026748757411, -0.016864118443, -0.078223266529, 0.266864118443, ...
                              0.602949018236, 0.266864118443, -0.078223266529, -0.016864118443, 0.026748757411];
lowSyn =  [-0.045635881557, -0.0287717631145, 0.295635881557,0.557543525, ...
                                   0.295635881557, -0.0287717631145, -0.045635881557];
highAna = [0.045635881557, -0.0287717631145, -0.295635881557, ...
                              0.557543525, -0.295635881557, -0.0287717631145, 0.045635881557];
highSyn = [0.026748757411, 0.016864118443, -0.078223266529, -0.266864118443, ...
                            0.602949018236, -0.266864118443, -0.078223266529, 0.016864118443, 0.026748757411];
             
% --- Number of channels
nChannels = 3;

currSubband = lena;
% ========================= Analysis Filter Bank ========================= %
for f = 1:nChannels
   
    [R,C] = size(currSubband);
    high = zeros(R,C/2);
    low = zeros(R,C/2);

    % --- Rows
    for r = 1:R
        [low(r,:),high(r,:)] = analysisTwoChannelFB(currSubband(r,:),lowAna,highAna);

    end

    lowlow = zeros(C/2, R/2);
    lowhigh = zeros(C/2, R/2);
    low_t = low.';
    
    % --- Columns
    for r = 1:C/2
        [lowlow(r,:),lowhigh(r,:)] = analysisTwoChannelFB(low_t(r,:),lowAna,highAna);           
    end

    highlow = zeros(C/2, R/2);
    highhigh = zeros(C/2, R/2);
    high_t = high.';
    
    for r = 1:C/2
        [highlow(r,:),highhigh(r,:)] = analysisTwoChannelFB(high_t(r,:),lowAna,highAna);           
    end
    
    if f==1
        transform = [lowlow.', highlow.'; lowhigh.', highhigh.'];
    else
        transform(1:R, 1:C) =  [lowlow.', highlow.'; lowhigh.', highhigh.'];
    end
    
    currSubband = lowlow.';
    
end
imshow(round(transform), [-128,127]);
R = R/2;
C = C/2;
% ========================= Synthesis Filter Bank ========================= %
for f = 1:nChannels
    

    % --- Columns
    currLow = transform(1:R,1:R)';
    currHigh = transform(R+1:2*R,1:R)';
    for  c = 1:C
        transform(1:2*R,c) = synthesisTwoChannelFB(currLow(c,:),currHigh(c,:),lowSyn,highSyn);
    end
   
    currLow = transform(1:R,R+1:2*(R))';
    currHigh = transform(R+1:2*R,R+1:2*(R))';
    for r = 1:R
       transform(1:2*R,r+R) = synthesisTwoChannelFB(currLow(r,:),currHigh(r,:),lowSyn,highSyn);
    end
   
    % --- Rows
    R = R*2;
    C = C*2;
    currLow = transform(1:R,1:C/2);
    currHigh = transform(1:R,(C/2) + 1 : C );
    for r = 1:R
       transform(r,1:R) = synthesisTwoChannelFB(currLow(r,:),currHigh(r,:),lowSyn,highSyn);
    end


end
imshow(round(transform), [-128,127]);


% --- Utility Functions
function x = synthesisTwoChannelFB(low,high,h,g)

    % --- Low pass channel
    % Upsampling
    lowUp = zeros(1,2*length(low));
    lowUp(1:2:end) = low;
    lowTemp = [fliplr(lowUp(2:end)), lowUp, fliplr(lowUp(1:end-1))];
    % Filter
    x0 = conv(lowTemp,h);
    addOnLen = length(fliplr(lowUp(2:end)));
    x0 = x0(((length(h)+1)/2)+addOnLen:((length(h)+1)/2)+2*addOnLen);
  
    
    % --- High pass channel
    % Upsampling
    highUp = zeros(1,2*length(high));
    highUp(1:2:end) = high;
    highTemp = [fliplr(highUp(2:end)), highUp, fliplr(highUp(1:end-1))];
    % Filter
    x1 = conv(highTemp,g);
    addOnLen = length(fliplr(highUp(2:end)));
    x1 = x1((length(g)-1)/2+addOnLen:(length(g)-1)/2+length(highUp)+addOnLen-1); 

    x = 2*(x0 + x1);
end


function [low,high] = analysisTwoChannelFB(x,h,g)

    xtemp = [fliplr(x(2:end)), x, fliplr(x(1:end-1))];
    addOnLen = length(x(2:end));
    % --- Low pass channel
    % Filter
    low = conv(xtemp,h);
    low = low(((length(h)+1)/2)+addOnLen:((length(h)+1)/2)+2*addOnLen);
    % Downsampling
    low = low(1:2:length(low));
    
    % --- High pass channel
    % Filter
    high = conv(xtemp,g);
    high = high((length(g)-1)/2+2+addOnLen:(length(g)-1)/2+length(x)+addOnLen+1);
    % Downsampling
    high = high(1:2:length(high));

end




      