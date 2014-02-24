clear all
clc

import java.net.ServerSocket
import java.io.*

server_socket = [];
input_socket = [];
input_stream = [];
jp2Size = 0;
port = 4567;


[server_socket input_socket] = function_TCPSetup(port);

input_stream   = input_socket.getInputStream;
d_input_stream = DataInputStream(input_stream);


period = [];

while(1)
    jp2Size = 0;
    try
        for n=1:10

            period(n) = 0;
            for i =0:3
                period(n) = period(n) + bitsll(uint32(d_input_stream.readUnsignedByte()),uint8( 8*(3-i) ) );
            end

        end

        for i =0:3
            jp2Size = jp2Size + bitsll(uint32(d_input_stream.readUnsignedByte()),uint8(8*i) );
        end

        
        for recv_idx = 1:jp2Size
            %{
            if(d_input_stream.readByte() == -1)
                display('connection close');
                break;
            end


            bytes_available = input_stream.available;


            for i = 1:bytes_available
                tmp_message(i) = d_input_stream.readByte();
            end
            %}
            
            message(recv_idx) =  uint8(d_input_stream.readUnsignedByte());
        end

    catch EOFException
        jp2Size = 0;
        display('close');
        break;

    end
    
        plot(message);
        drawnow;

    %message = reshape(message', 1, []);
end




function_TCPRelease(server_socket, input_socket);
display('Client disconnected');