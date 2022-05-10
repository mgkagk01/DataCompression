classdef Encoder
    

    properties
        nFrames;
        video;
        macroSize = 8;
        width;
        height;
        nMacroblocks;
        seachAreaStep = 4;
        codingOrder; 
        nBframes;
        motionVectors; 
        residaulFrames;
        dctCoefficientFrames;
        maxNumBframes = 2
        buffer; % To store the B-Frames
        bufferSize; % To count the # of B-Frames stored in buffer
        
        dct;
        invdct;
        % --- Parameters of the Quantizer
        QS = 20; % Quantizer Scale
        intraQ = [8 16 19 22 26 27 29 34; 
                  16 16 22 24 27 29 34 37;
                  19 22 26 27 29 34 34 38;
                  22 22 26 27 29 34 37 40;
                  22 26 27 29 32 35 40 48;
                  26 27 29 32 35 40 48 58;
                  26 27 29 34 38 46 56 69;
                  27 29 35 38 46 56 69 83];
              
        nonIntraQ;
        quantizedValuesFrames;
               
                        
    end
    
    
    
    methods
        
        % === Constructor
        function obj = Encoder(videoInfo, video)
            obj.nFrames = videoInfo.frameCount;
            obj.width = videoInfo.width;
            obj.height = videoInfo.height;
            obj.video = video;
            obj.nMacroblocks = (obj.width * obj.height)/(obj.macroSize * obj.macroSize);
            obj.codingOrder = [repmat([0 repmat([2 2 1],1,3)],1,11), 2, 1];
            obj.nBframes = length(find(obj.codingOrder == 2));
            obj.motionVectors = zeros(2,obj.nMacroblocks,obj.nFrames + obj.nBframes);
            obj.residaulFrames = zeros(obj.height,obj.width,obj.nFrames);
            obj.dctCoefficientFrames = zeros(obj.height,obj.width,obj.nFrames);
            obj.quantizedValuesFrames = zeros(obj.height,obj.width,obj.nFrames);
            obj.buffer = zeros(obj.height,obj.width,obj.maxNumBframes);
            obj.bufferSize = 0;
            obj.dct = @(block_struct) dctmtx(8) * block_struct.data * dctmtx(8)';
            obj.invdct = @(block_struct) dctmtx(8)' * block_struct.data *  dctmtx(8);
            obj.intraQ = obj.intraQ * obj.QS;
            obj.nonIntraQ = ones(obj.macroSize,obj.macroSize)* (16 * obj.QS);
        end
        
        
        % === Encoder
        function [qValues,moVe] = encode(obj)
                       
            shift = 0;
            mvIdx = 0;
            flag =0;
            % ---- For all frames
            for i = 1:obj.nFrames
                
                % --- From RGB to YCbCr
                videoFrame = obj.video(i);
                YCbCr = double(rgb2ycbcr(videoFrame.cdata));
                
                % --- Encode only the luminance to make the simulation
                % faster
                frame = YCbCr(:,:,1)-128;
                
                % --- Check the type of the frame
                if obj.codingOrder(i) == 0 % I - Frame
                    mvIdx = mvIdx + 1;
                    % --- Discrete Cosine Transform (DCT)
                    obj.dctCoefficientFrames(:,:,i) = dctTransform(obj,frame);

                    % --- Quantization 
                    obj.quantizedValuesFrames(:,:,i) = quantizer(obj,obj.dctCoefficientFrames(:,:,i),0);
                    
                    % --- Inverse Quantization
                    dctCoefficients = invQuantizer(obj,obj.quantizedValuesFrames(:,:,i),0);
                    
                    % --- Inverse DCT
                    residual = invdctTransform(obj,dctCoefficients);
                    
                    currForward = residual;
                    obj.residaulFrames(:,:,i) = residual;
                    
                    
                    
                elseif obj.codingOrder(i) == 1 % P - Frame
                    mvIdx = mvIdx + 1;
                    
                    % === Forward Prediction
                    obj.motionVectors(:,:,mvIdx) = framePprediction(obj,currForward,frame);
                    
                    % === Compute the residual
                    % --- Motion Compensation
                    frameHat = motionCompensation(obj,currForward,obj.motionVectors(:,:,mvIdx));
                    
                    % --- Compute Residual
                    obj.residaulFrames(:,:,i) = frame - frameHat;
                    
                    %plotting motion vectors using Quiver

                     
                    % --- Discrete Cosine Transform (DCT)
                    obj.dctCoefficientFrames(:,:,i) = dctTransform(obj,obj.residaulFrames(:,:,i));

                    % --- Quantization 
                    obj.quantizedValuesFrames(:,:,i) = quantizer(obj,obj.dctCoefficientFrames(:,:,i),1);
                    
                    % --- Inverse Quantization
                    dctCoefficients = invQuantizer(obj,obj.quantizedValuesFrames(:,:,i),1);
                    
                    % --- Inverse DCT
                    residualHat = invdctTransform(obj,dctCoefficients);
                    
                    
                    if obj.bufferSize == 0 % The currect P-frame will be use for Forward prediction
                        currForward = frameHat + residualHat;
                        
                    elseif obj.bufferSize == obj.maxNumBframes % The currect P-frame will be use for Backward prediction
                        currBackward = frameHat + residualHat;
                        obj.bufferSize = obj.bufferSize  + 1;
                        
                    elseif obj.bufferSize < obj.maxNumBframes
                        currBackward = frameHat + residualHat;
                        flag = 1;
                    end
                    
                    
                    
                     
                else % B - Frame
                    % --- Store the B frames and encoded them when you
                    % encode the next "Backward Frame" , i.e. I or P-frame
                    obj.bufferSize = obj.bufferSize + 1;
                    obj.buffer(:,:,obj.bufferSize) = frame;
                    mvIdx = mvIdx + 2;
                    shift = shift + 2;
                end
                
                
                % === Encode B-Frames                
                if obj.bufferSize == obj.maxNumBframes + 1 || flag == 1
                   % === Encode the B-Frames
                   for b = 1:obj.bufferSize - 1 + flag
                       
                      % === Motion Estimation                       
                      % --- Find the motion vectors using Forward Prediction
                      obj.motionVectors(:,:,mvIdx-shift) = framePprediction(obj,currForward,obj.buffer(:,:,b));
                      % --- Find the motion vectors using Backward Prediction
                      obj.motionVectors(:,:,mvIdx-shift+1) = framePprediction(obj,currBackward,obj.buffer(:,:,b));
                                            
                      % === Motion Compensation
                      % --- Forward
                      frameHatForward = motionCompensation(obj,currForward,obj.motionVectors(:,:,mvIdx-shift));                      
                      % --- Backward
                      frameHatBackward = motionCompensation(obj,currBackward,obj.motionVectors(:,:,mvIdx-shift+1));
                      % --- Find the average
                      frameHat = (frameHatForward + frameHatBackward)/2;
                      
                      % === Compute Residual
                      obj.residaulFrames(:,:,i-obj.bufferSize+b-flag) = obj.buffer(:,:,b) - frameHat;
                      
                      % === Discrete Cosine Transform (DCT)
                      obj.dctCoefficientFrames(:,:,i-obj.bufferSize+b-flag) = dctTransform(obj,obj.residaulFrames(:,:,i-obj.bufferSize+b-flag));
                      
                      % === Quantization
                      obj.quantizedValuesFrames(:,:,i-obj.bufferSize+b-flag) = quantizer(obj,obj.dctCoefficientFrames(:,:,i-obj.bufferSize+b-flag),1);
                      
                      
                      shift = shift - 2;
                   end
                   if flag
                       flag = 0;
                   end
                   obj.bufferSize = 0; % Indicate that the buffer is empty
                   currForward = currBackward; % Set the latest P-frame to be the next frame to use for forward prediction  
                end
                

                
                
            end
            
        qValues = obj.quantizedValuesFrames;
        moVe = obj.motionVectors;
        
        end
        
               
        
        % =============== Functions For DCT Tranform =============== %
        % === DCT Transform
        function dctCoefficients = dctTransform(obj,residual)
            
            dctCoefficients = zeros(obj.height,obj.width);
            m = 1;
            for h = 1:obj.height/obj.macroSize
                for w = 1:obj.width/obj.macroSize
                    r1 =  (h-1)*obj.macroSize + 1;
                    r2 = h*obj.macroSize;
                    c1 = (w-1)*obj.macroSize + 1;
                    c2 = w*obj.macroSize;

                    dctCoefficients(r1:r2, c1:c2) = blockproc(residual(r1:r2, c1:c2),[8 8], obj.dct);                    
                    
                    m = m + 1;
                end
            end
            
         end
        

        
        % =============== Functions For Quantizer =============== %
        function quantizedValues = quantizer(obj,dctCoefficients,type)
            
            quantizedValues = zeros(obj.height,obj.width);
            
            % --- Step 1: Select Quantizer
            if type == 0 % I-Frame 
                Q = obj.intraQ;
            else % B or P-Frame
                Q = obj.nonIntraQ;
            end
            
            % --- Step 2: Multiply the DCT coefficients by 16
            dctCoefficients = dctCoefficients * 16;
            
            
            for h = 1:obj.height/obj.macroSize
                for w = 1:obj.width/obj.macroSize
                    r1 =  (h-1)*obj.macroSize + 1;
                    r2 = h*obj.macroSize;
                    c1 = (w-1)*obj.macroSize + 1;
                    c2 = w*obj.macroSize;
                    
                                       
                    % --- Step 3: Quantize
                    quantizedValues(r1:r2, c1:c2) = round(dctCoefficients(r1:r2, c1:c2)./Q);

                end
            end
            
        end
        
        

        
        
        % =============== Functions For Motion Estimation =============== %
        % === Find the motion Vectors (Intre Prediction)
        function motionVectors = framePprediction(obj,I,P)
            motionVectors = zeros(2, obj.nMacroblocks);
            [r,c] = size(I);
            costs = ones(2*obj.seachAreaStep + 1, 2*obj.seachAreaStep +1) * inf;
            % --- For all macroblocks
            m = 1;
            for h = 1:obj.height/obj.macroSize
                for w = 1:obj.width/obj.macroSize
                    r1 =  (h-1)*obj.macroSize + 1;
                    r2 = h*obj.macroSize;
                    c1 = (w-1)*obj.macroSize + 1;
                    c2 = w*obj.macroSize;
                    
                    currMacroblk = P(r1:r2, c1:c2);
                    
                    minCost = Inf;
                    
                    % --- For all the blocks in the search area
                    for vertical = -obj.seachAreaStep : obj.seachAreaStep
                        for horizontal = -obj.seachAreaStep : obj.seachAreaStep
                            
                            if(r1 + vertical < 1 || c1 + horizontal < 1 || r2 + vertical > r || c2 + horizontal > c)
                                continue;
                            else
                                % --- Compute the Absolute Difference
                                currSearchArea = I(r1 + vertical:r2+vertical, c1+horizontal:c2+horizontal);
                                costs(vertical+obj.seachAreaStep+1,horizontal+obj.seachAreaStep+1) = compuCost(obj,currSearchArea,currMacroblk);
                                if minCost > costs(vertical+obj.seachAreaStep+1,horizontal+obj.seachAreaStep+1)
                                    minCost =  costs(vertical+obj.seachAreaStep+1,horizontal+obj.seachAreaStep+1);
                                    motionVectors(1,m) = vertical;
                                    motionVectors(2,m) = horizontal;
                                    
                                end
                            end
                            
                        end
                    end
                    m = m + 1;
                end
            end
             
        end
        
        
        % === Compute the Absolute Cost
        function cost = compuCost(obj,x,y)
            cost = 0;
            for i = 1:obj.macroSize
                for j = 1:obj.macroSize
                    cost = cost + abs(double(x(i,j) - y(i,j)));
                end
            end
            
            cost = cost / (obj.macroSize ^2);
        end
        
        
       
        
        
        
        
        
        
        % ==================================================================================== % 
        % =========================== Functions of Embedded Decoder =========================== %
        % ==================================================================================== % 
        
        % ==== STEP 1: Inverse Quantization
        function dctCoefficients = invQuantizer(obj,quantizedValues,type)
            
            dctCoefficients = zeros(obj.height,obj.width);
            
            % --- Step 1: Select Quantizer
            if type == 0 % I-Frame
                Q = obj.intraQ;
            else % B or P-Frame
                Q = obj.nonIntraQ;
            end
                        
            for h = 1:obj.height/obj.macroSize
                for w = 1:obj.width/obj.macroSize
                    r1 =  (h-1)*obj.macroSize + 1;
                    r2 = h*obj.macroSize;
                    c1 = (w-1)*obj.macroSize + 1;
                    c2 = w*obj.macroSize;
                    
                    
                    % --- Step 2: Quantize
                    dctCoefficients(r1:r2, c1:c2) = (quantizedValues(r1:r2, c1:c2).*Q) / 16;
                end
            end
        end
        
        
        
        % ==== STEP 2: Inverse DCT Transform
        function residual = invdctTransform(obj,dctCoefficients)
            
            residual = zeros(obj.height,obj.width);
            
            for h = 1:obj.height/obj.macroSize
                for w = 1:obj.width/obj.macroSize
                    r1 =  (h-1)*obj.macroSize + 1;
                    r2 = h*obj.macroSize;
                    c1 = (w-1)*obj.macroSize + 1;
                    c2 = w*obj.macroSize;
                    
                    residual(r1:r2, c1:c2) = blockproc(dctCoefficients(r1:r2, c1:c2),[8 8],obj.invdct);

                    
                end
            end
            
        end
        
        
        
        % ==== STEP 3: Motion Compensation 
        function recFrame = motionCompensation(obj,refeFrame,motionVectors)
            % --- To store the recostructed Frame
            recFrame = zeros(obj.height,obj.width);
            m = 1;
            for h = 1:obj.height/obj.macroSize
                for w = 1:obj.width/obj.macroSize
                    % --- Coordinates
                    r1 =  (h-1)*obj.macroSize + 1;
                    r2 = h*obj.macroSize;
                    c1 = (w-1)*obj.macroSize + 1;
                    c2 = w*obj.macroSize;
                    
                    % --- Motion Vectors
                    vertical = motionVectors(1,m);
                    horizontal = motionVectors(2,m);
                    recFrame(r1:r2, c1:c2) = refeFrame(r1+vertical:r2+vertical, c1+horizontal:c2+horizontal);
                    
                    m = m +1;
                end
            end
        end
        
        
        
        
    end
    
    
end
