/*

file: socket.hpp
author: juska933
created: 2019-11-13

Wrapper class for socket communication.

*/

#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <vector>
#include <string>
#include <iostream>
#include <functional>

#include <json/json.hpp>


#define PORT 8000
#define ACTIVITY_DELAY_MICRO_SECONDS 0
//#define ACTIVITY_DELAY_MICRO_SECONDS 10000


using MessageHandler = std::function<void(std::string, int)>;
using JsonHandler = std::function<void(nlohmann::json, int)>;
class Socket {

public:
    Socket(int max_clients=30);

    void send_to_clients_json(nlohmann::json msg);
    void send_to_clients_json(std::string route, nlohmann::json msg);
    void send_to_clients(std::string msg);

    void on_message(MessageHandler message_handler);
    void on_json(JsonHandler json_handler);
    void start_socket();
    void check_activity();

private:
    void emit_message(int sd, std::string msg);

    sockaddr* cast_sock_addr();
    void try_accept_client();
    void check_incoming_message();

    int opt = 1;
    int master_socket , addrlen , new_socket;
    int max_clients, activity;
    std::vector<int> client_sockets;
    struct sockaddr_in address;
    char buffer[1025];  //data buffer of 1K

    fd_set readfds;

    std::vector<MessageHandler> message_handlers;
    std::vector<JsonHandler> json_handlers;
    std::string last_msg_buffer;
};


#endif /* SOCKET_H */
