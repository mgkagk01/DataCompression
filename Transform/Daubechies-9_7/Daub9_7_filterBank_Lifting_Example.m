clc 
clear all
% === Implementation of Daubechies 9/7 waveltet using lifting method with
% rounding

% --- Load Lenna
lena = imread('Lenna.png');
lena = double(rgb2gray(lena)) - 128;

% --- Define Lamda filters for Daubechies 9/7
lambdas = fliplr([0 -1.586134342 -1.586134342;
           -0.052980118 -0.052980118 0;
            0 0.882911075 0.882911075;
            0.443506852 0.443506852 0]);
        
K = 1.230174105;
K0 = 1/K;
K1 = K/2;

% --- To store the variance for each subband
variance = zeros(1,10);
count = 1;

% --- Number of Lifting Step
[nSteps,~] = size(lambdas);

% --- Number of channels
nChannels = 3;

currSubband = lena;
[R,C] = size(lena);

% ========================= Analysis Lifting ========================= %
for f = 1:nChannels
    
    [R,C] = size(currSubband);
    high = zeros(R,C/2);
    low = zeros(R,C/2);
    oIndex = 2:2:R;
    eIndex = 1:2:R-1;
    
    % --- Rows
    for r = 1:R
        [low(r,:),high(r,:)] = liftingAna(currSubband(r,eIndex),currSubband(r,oIndex),lambdas,nSteps,K0,K1);
    end

    % --- Columns
    lowT = low.';
    lowlow = zeros(C/2, R/2);
    lowhigh = zeros(C/2, R/2);
    for r = 1:C/2
        [lowlow(r,:),lowhigh(r,:)] = liftingAna(lowT(r,eIndex),lowT(r,oIndex),lambdas,nSteps,K0,K1);          
    end
    
    
    highT = high.';
    highlow = zeros(C/2, R/2);
    highhigh = zeros(C/2, R/2);
    for r = 1:R/2
        [highlow(r,:),highhigh(r,:)] = liftingAna(highT(r,eIndex),highT(r,oIndex),lambdas,nSteps,K0,K1);          
    end

 if f==1
        transform = [lowlow.', highlow.'; lowhigh.', highhigh.'];
    else
        transform(1:R, 1:C) =  [lowlow.', highlow.'; lowhigh.', highhigh.'];
 end
 

     % --- Compute the variances for bit allocation
     variance(count) = computeCovariance(highlow); 
     count = count +1;
     variance(count) = computeCovariance(lowhigh); 
     count = count +1;
     variance(count) = computeCovariance(highhigh); 
     count = count +1;
     

currSubband = lowlow.';
end
variance(count) = computeCovariance(lowlow); 


imshow(round(transform), [-128,127]);

% --- Bit allocation
bits =  bitAlloc(variance, 512*512, 10);

% ========================= Synthesis Lifting ========================= %
R = R/2;
C = C/2;
for f = 1:nChannels
    

    % --- Columns
    currLow = transform(1:R,1:R);
    currHigh = transform(R+1:2*R,1:R);
    oIndex = 2:2:2*R;
    eIndex = 1:2:2*R-1;
    for  c = 1:C
        [transform(eIndex,c), transform(oIndex,c)] = liftingSyn(currLow(:,c)',currHigh(:,c)',lambdas,nSteps,K0,K1);
    end
    
    currLow = transform(1:R,R+1:2*(R));
    currHigh = transform(R+1:2*R,R+1:2*(R));
    for r = 1:R
       [transform(eIndex,r+R), transform(oIndex,r+R)] = liftingSyn(currLow(:,r)',currHigh(:,r)',lambdas,nSteps,K0,K1);


    end

    % --- Rows
    R = R*2;
    C = C*2;
    currLow = transform(1:R,1:C/2);
    currHigh = transform(1:R,(C/2) + 1 : C );
    oIndex = 2:2:R;
    eIndex = 1:2:R-1;
    for r = 1:R
       [transform(eIndex,r), transform(oIndex,r)] = liftingSyn(currLow(r,:),currHigh(r,:),lambdas,nSteps,K0,K1);
    end
    transform(1:R,1:C) = transform(1:R,1:C)';

end

imshow(round(transform), [-128,127]);


function [evenOut,oddOut] = liftingSyn(even,odd,lambdas,nSteps,K0,K1)

    even = even/K0;
    odd = odd/K1;
    for i = nSteps:-1:1
        [even,odd] =  liftingStep(even,odd,lambdas(i,:),i,-1);
    end
    
    evenOut = even;
    oddOut = odd;
end

function [evenOut,oddOut] = liftingAna(even,odd,lambdas,nSteps,K0,K1)
    for i = 1:nSteps
        [even,odd] =  liftingStep(even,odd,lambdas(i,:),i,1);
    end
    
    evenOut = K0*even;
    oddOut = K1*odd;
end

function [evenOut,oddOut] =  liftingStep(even,odd,lambda,i,indicator)
        if mod(i,2) % Odd
            evenOut = even;
            evenTemp = [fliplr(even(2:end)), even, fliplr(even(1:end-1))];
            addOnLen = length(even(2:end));
           
            % Filter
            evenF= conv(evenTemp,lambda);
            evenF = round(evenF(((length(lambda)+1)/2)+addOnLen:((length(lambda)+1)/2)+2*addOnLen));
            
            oddOut = odd + indicator*evenF;
        else % even
            oddOut = odd;
            oddTemp = [fliplr(odd(2:end)), odd, fliplr(odd(1:end-1))];
            addOnLen = length(oddOut(2:end));
           
            % Filter
            oddF = conv(oddTemp,lambda);
            oddF = round(oddF(((length(lambda)+1)/2)+addOnLen:((length(lambda)+1)/2)+2*addOnLen));
            
            evenOut = even + indicator*oddF;
        end
end