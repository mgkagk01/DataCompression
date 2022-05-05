classdef Decoder
    
    properties
        nFrames;
        video;
        macroSize = 8;
        width;
        height;
        codingOrder; 
        nBframes;
        buffer;
        bufferSize;
        maxNumBframes = 2
        invdct;
         QS = 1; % Quantizer Scale
        intraQ = [8 16 19 22 26 27 29 34; 
                  16 16 22 24 27 29 34 37;
                  19 22 26 27 29 34 34 38;
                  22 22 26 27 29 34 37 40;
                  22 26 27 29 32 35 40 48;
                  26 27 29 32 35 40 48 58;
                  26 27 29 34 38 46 56 69;
                  27 29 35 38 46 56 69 83];
              
        nonIntraQ;
    
    end
    
    
    
    methods
        
        % === Constructor
        function obj = Decoder(videoInfo)
            obj.nFrames = videoInfo.frameCount;
            obj.width = videoInfo.width;
            obj.height = videoInfo.height;
            obj.video = zeros(obj.height,obj.width,obj.nFrames);
            obj.codingOrder = [repmat([0 repmat([2 2 1],1,3)],1,11), 2, 1];
            obj.nBframes = length(find(obj.codingOrder == 2));
            obj.buffer = zeros(obj.height,obj.width,obj.maxNumBframes);
            obj.bufferSize = 0;
            obj.invdct = @(block_struct) dctmtx(8)' * block_struct.data *  dctmtx(8);
            obj.intraQ = obj.intraQ * obj.QS;
            obj.nonIntraQ = ones(obj.macroSize,obj.macroSize)* (16 * obj.QS);

        end
        
        
        % === Decoder
        function decode(obj,quantizedValues,motionVectors)
            
            shift = 0;
            mvIdx = 0;
            flag = 0;
            % ---- For all frames
            for i = 1:obj.nFrames
                
                % --- Inverse Quantization
                if obj.codingOrder(i) == 0
                    dctCoefficients = invQuantizer(obj,quantizedValues(:,:,i),0);
                else
                    dctCoefficients = invQuantizer(obj,quantizedValues(:,:,i),1);
                end
                
                % ---- Inverse DCT Transform
                residual = invdctTransform(obj,dctCoefficients);

                
                % --- Check the type of the frame
                if obj.codingOrder(i) == 0 % I - Frame
                    % --- If it is a I-Frame => not Intre prediction
                    obj.video(:,:,i) = residual;
                    currForward = residual;
                    mvIdx = mvIdx + 1;

                    
                elseif obj.codingOrder(i) == 1 % P - Frame
                    mvIdx = mvIdx + 1;
                    % --- Motion Compensation
                    frameHat = motionCompensation(obj,currForward,motionVectors(:,:,mvIdx));
                    
                    
                    % --- Add the residual
                    obj.video(:,:,i) = frameHat + residual;
                    
                    if obj.bufferSize == 0 % The currect P-frame will be use for Forward prediction
                        currForward = obj.video(:,:,i);
                        
                    elseif obj.bufferSize == 2 % The currect P-frame will be use for Backward prediction
                        currBackward = obj.video(:,:,i);
                        obj.bufferSize = obj.bufferSize + 1;
                        
                    elseif obj.bufferSize < obj.maxNumBframes
                        currBackward = obj.video(:,:,i);
                        flag = 1;
                    end
                    

                    
                    
                else % B - Frame
                    obj.bufferSize = obj.bufferSize + 1;
                    obj.buffer(:,:,obj.bufferSize) = residual;
                    mvIdx = mvIdx + 2;
                    shift = shift + 2;

                end
                
                % === Decode B-Frames  
                if obj.bufferSize == obj.maxNumBframes+1 || flag == 1
                     for b = 1:obj.bufferSize-1+ flag
                        % === Motion Compensation
                        % --- Forward Prediction
                        frameHatForward = motionCompensation(obj,currForward,motionVectors(:,:,mvIdx-shift));
                        % --- Backward Prediction
                        frameHatBackward = motionCompensation(obj,currBackward,motionVectors(:,:,mvIdx-shift+1));
                        % --- Find the average
                        frameHat = (frameHatForward + frameHatBackward)/2;
                        
                        % ---Add the Residual
                        obj.video(:,:,i-obj.bufferSize+b-flag) =  frameHat + obj.buffer(:,:,b);
                        shift = shift - 2;
                     end
                     obj.bufferSize = 0;
                     currForward = currBackward;
                end   
                if flag
                    flag = 0;
                end
            end
            
            
            
            
          playVideo(obj);
       
        end
        
        
        function playVideo(obj)
            
            for i = 1:obj.nFrames
                image(obj.video(:,:,i)+128);
                pause(0.1)
            end
            
        end
        
        % =============== Inverse DCT Transform =============== %
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
        
        % =============== Functions For Quantizer =============== %
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
        
        
      
        
        % =============== Functions For Motion Compensation =============== %
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


