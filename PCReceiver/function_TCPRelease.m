function function_TCPRelease(server_socket, input_socket)
    import java.net.ServerSocket
    import java.io.*
    
    server_socket.close;
    input_socket.close;
end