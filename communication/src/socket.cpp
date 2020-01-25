/*

file: socket.cpp
author: juska933
created: 2019-11-13

Wrapper class for socket communication.

*/

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

#include <json/json.hpp>

#include "socket.hpp"
#include "logging.hpp"


using json = nlohmann::json;


static std::vector<std::string>
split_str(const std::string &text, std::string delim)
{
    std::vector<std::string> res;
    size_t i = 0, len = delim.length();
    size_t prev = 0;
    while (i+len <= text.length()) {
		if (text.substr(i, len) == delim) {
			res.push_back(text.substr(prev, i-prev));
			i += len;
			prev = i;
		} else {
			i++;
		}
	}
	res.push_back(text.substr(prev, text.size()-prev));
	return res;
}


Socket::Socket(int max_clients)
{
    this->max_clients = max_clients;
    client_sockets.assign(max_clients, 0);
}

void
Socket::send_to_clients_json(json msg)
{
    send_to_clients(msg.dump());
}

void Socket::send_to_clients_json(std::string route, json msg)
{
    json patch = {{"route", route}};
    msg.merge_patch(patch);
    send_to_clients(msg);
}


void Socket::send_to_clients(std::string msg){
    //check_incoming_message();
    msg = msg + "__MSG_END__";
    for (unsigned i = 0; i < client_sockets.size(); i++) {
        int client = client_sockets[i];
        if (client == 0)
            continue;
        if( send(client, msg.c_str(), msg.length(), 0) != (ssize_t)msg.length() ){
            WARN("Could not send message to client");
            client_sockets[i] = 0;
        }
    }
}

void Socket::emit_message(int sd, std::string msg){
    std::string entire_msg = last_msg_buffer + msg;
    std::vector<std::string> packets = split_str(entire_msg, "__MSG_END__");
    std::string last_msg = packets[packets.size()-1];
    if (last_msg.size() > 0) {
        last_msg_buffer = last_msg; // Save partial message
    } 
    packets.pop_back();
    for (MessageHandler message_handler: message_handlers) {
        for (std::string packet: packets)  {
            message_handler(packet, sd);
        }

    }
    std::vector<json> json_packets;
    for (std::string packet: packets) {
        try {
            json data = json::parse(packet);
            json_packets.push_back(data);
        } catch (const nlohmann::detail::parse_error& e) {
            WARN("Incomplete json message from socket: " + packet);
        }
    }
    for (JsonHandler json_handler: json_handlers) {
        for (json packet: json_packets)  {
            json_handler(packet, sd);
        }

    }
}

void Socket::on_message(MessageHandler message_handler) {
    this->message_handlers.push_back(message_handler);
}

void Socket::on_json(JsonHandler json_handler){
    this->json_handlers.push_back(json_handler);
}

void Socket::start_socket(){
    //create a master socket
    WARN("STARTING SOCKET");
    master_socket = socket(AF_INET , SOCK_STREAM , 0);
    if (!master_socket) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    int multiple_socket_res = setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    if(multiple_socket_res < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8000
    if (bind(master_socket, cast_sock_addr(), sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
}

void Socket::check_activity(){
    //clear the socket set
    FD_ZERO(&readfds);

    //add master socket to set
    FD_SET(master_socket, &readfds);
    int max_sd = master_socket;

    //add child sockets to set
    for (int i = 0 ; i < max_clients ; i++)
    {
        //socket descriptor
        int sd = client_sockets[i];

        //if valid socket descriptor then add to read list
        if(sd > 0)
            FD_SET( sd , &readfds);

        //highest file descriptor number, need it for the select function
        if(sd > max_sd)
            max_sd = sd;
    }

    //wait for an activity on one of the sockets , timeout is NULL (last arg),
    //so wait indefinitely
    // NOTE: Might be problem when integrating bcz of stalling
    timeval timeout = {0, ACTIVITY_DELAY_MICRO_SECONDS};
    activity = select( max_sd + 1 , &readfds , NULL , NULL , &timeout);

    if ((activity < 0) && (errno!=EINTR)) {
        return;
    }
    try_accept_client();
    check_incoming_message();
}

sockaddr* Socket::cast_sock_addr(){
    return (struct sockaddr *) &address;
}


void Socket::try_accept_client(){
    //If something happened on the master socket ,
    //then its an incoming connection
    if (FD_ISSET(master_socket, &readfds))
    {
        if ((new_socket = accept(master_socket,  cast_sock_addr(), (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        //inform user of socket number - used in send and receive commands
        printf("New connection , socket fd is %d , ip is : %s , port : %d  \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

        //send new connection greeting message


        //add new socket to array of sockets
        for (int i = 0; i < max_clients; i++)
        {
            //if position is empty
            if( client_sockets[i] == 0 )
            {
                client_sockets[i] = new_socket;
                break;
            }
        }
    }
}

void Socket::check_incoming_message(){
    for (int i = 0; i < max_clients; i++)
    {
        int sd = client_sockets[i];

        if (FD_ISSET( sd , &readfds))
        {
            //Check if it was for closing , and also read the
            //incoming message
            int num_bytes_read = read( sd , buffer, 1024);
            if (num_bytes_read <= 0)
            {
                //Somebody disconnected , get his details and print
                getpeername(sd , cast_sock_addr(), (socklen_t*)&addrlen);
                printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                //Close the socket and mark as 0 in list for reuse
                close(sd);
                client_sockets[i] = 0;
            }

            //Echo back the message that came in
            else
            {
                //set the string terminating NULL byte on the end
                //of the data read
                //buffer[num_bytes_read] = '\0';

                

                //printf("msg incoming: %s\n", buffer);
                emit_message(sd, std::string(buffer, num_bytes_read));
            }
        }
    }
}
