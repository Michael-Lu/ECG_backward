function [server_socket input_socket] = function_TCPSetup(port)
    import java.net.ServerSocket
    import java.io.*

    server_socket  = [];
    input_socket  = [];
    
    fprintf(1, ['Try waiting for client to connect to this ' ...
                        'host on port : %d\n'], port);
    while true
        try
            server_socket = java.net.ServerSocket(port);
            server_socket.setSoTimeout(10000);

            input_socket = server_socket.accept;

            fprintf(1, 'Client connected\n');       
       
            break;
        catch
            if ~isempty(server_socket)
                server_socket.close;
            end

            if ~isempty(input_socket)
                input_socket.close;
            end
            pause(1);
        end
        
   end

end