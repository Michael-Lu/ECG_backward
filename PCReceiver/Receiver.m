clear all;
err = Jp2Decoder('init');
if(err)
    err
else
   figure;
   while(1)
       [raw_data, period, features, err] = Jp2Decoder('receive');
       if(err)
           err
           break;
       end
       
       if(size(raw_data) == [0 0]) %Transfer is done! Automatically call jp2Decoder('close')
           break;
       end
       
       display('----period----');
       period
       display('------------');
       features
       display('------------');
       
       [height width] = size(raw_data);
       de_norm_data = [];
       for i = 1:height
           de_norm_data = cat(2,de_norm_data, interp1(1:( (double(period(i))-1)/(width-1) ):double(period(i)), double(raw_data(i,:)), 1:double(period(i)),'linear') );
       end
       
       plot(de_norm_data);
       drawnow;
   end
end

err = Jp2Decoder('close');
if(err)
    err
end