clc 
clear all


% === Toy example to check Run length Coding
lenX = 64;
EOB = [-1,-1];

% --- Signal to be encoded
x = [124 23 18 16 2 1 0 6 -1 1 0 0 0 1 1];
x = [x zeros(1,lenX  - length(x))];

% --- Encode
y = rlcEncoder(x, lenX, EOB);

% --- Decode
xHat = rlcDecoder(y, lenX, EOB);

if sum(abs(x-xHat)) > 0
    disp('Problem');
else
    disp('Success');
end


function y = rlcEncoder(x, lenX, EOB)
    y = zeros(lenX,2);
    j = 1;
    countZeros = 0;
    for i = 1:lenX
       if x(i) == 0
            countZeros = countZeros + 1;
       else
           y(j,1) = countZeros;
           y(j,2) = x(i);
           j = j + 1;
           countZeros = 0;
       end
    end

    if countZeros > 0
        y(j,:) = EOB;
    end
    
    
    y = y(1:j,:);
end


function x = rlcDecoder(y, lenX, EOB)
    
    x = zeros(1,lenX);
    [r,~]  = size(y);
    j = 1;
    for i = 1:r
        if sum(abs(y(i,:) - EOB)) == 0
            break;
        else
            shiftZeros = y(i,1);
            x(j+shiftZeros) = y(i,2); 
            j = j + shiftZeros + 1;
        end        
    end
end

    
    