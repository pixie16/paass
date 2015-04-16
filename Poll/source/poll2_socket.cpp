#include "poll2_socket.h"

struct sockaddr_in cli_sa, cli_addr, serv_sa;
bool cli_init(false), serv_init(false);
int cli_sock, serv_sock;
socklen_t cli_size, serv_size;

bool client_connected = false;

bool init_client(const char* address_, int port_){
	if(cli_init){ return false; }

	if((cli_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){ 
		return false;
	}
	
	bzero(&cli_sa, sizeof cli_sa);
	cli_sa.sin_family = AF_INET;

	int numbers[4];
	std::string temp;
	unsigned int count = 0, index = 0;
	
	while(true){
		if(address_[count] == '.'){
			numbers[index] = atoi(temp.c_str());
			temp = "";
			index++;
		}
		else{ temp += address_[count]; }
		count++;
	
		if(address_[count] == '\0' || index >= 4){ 
			numbers[index] = atoi(temp.c_str());
			break; 
		}
	}

	cli_sa.sin_port = htons(port_);
	cli_sa.sin_addr.s_addr = htonl((((((numbers[0] << 8) | numbers[1]) << 8) | numbers[2]) << 8) | numbers[3]);		

	/*if(connect(cli_sock, (struct sockaddr *)&cli_sa, sizeof cli_sa) < 0){
		close(cli_sock);
		return false;
	}*/

	cli_size = sizeof(cli_sa);

	return cli_init = true;
}

/*bool client_send_message(char *message_, size_t length_){
	if(!cli_init){ return false; }
	
	if(write(cli_sock, message_, length_) < 0){
		return false; 
	}

	return true;
}

bool client_send_message(std::string message_){
	char input[message_.size()];
	for(size_t i = 0; i < message_.size(); i++){
		input[i] = message_[i];
	}
	
	return client_send_message(input, message_.size());
}*/

int client_recv_message(char *message_ , size_t length_){
	if(!cli_init){ return false; }

	// Clear the message
	bzero(message_, length_);

	/*if(read(cli_sock, message_, length_) < 0){
		return false;
	}*/
	
	return recvfrom(cli_sock, message_, length_, 0, (struct sockaddr *) &cli_sa, &cli_size);
}

int client_recv_message(std::string &message, size_t max_length_){
	std::stringstream stream;
	char str[max_length_];
	int output = client_recv_message(str, max_length_);
	
	stream << str;
	message = stream.str();
	return output;
}

bool close_client(){
	if(!cli_init){ return false; }
	
	close(cli_sock);
	return true;
}

bool init_server(int portNum_){
	if(serv_init){ return false; }

	if((serv_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		return false;
	}
	
	bzero(&serv_sa, sizeof serv_sa);
	serv_sa.sin_family = htonl(AF_INET);
	serv_sa.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_sa.sin_port = htons(portNum_);
	
	if(bind(serv_sock, (struct sockaddr *) &serv_sa, sizeof(serv_sa)) < 0){ 
		close(serv_sock);
		return false;
	}
	
	//listen(serv_sock, 5);
	
	serv_size = sizeof(serv_sa);
	
	return serv_init = true;
}

/*bool run_server(){
	if(!serv_init){ return false; }

	char buffer[512];
	char reply[19] = "I got your message";
	size_t size;
	int nbytes;

	unsigned int cli_addr_len = sizeof(cli_addr);
	size = sizeof (serv_sa);
	
	while(true){
		cli_sock = accept(serv_sock, (struct sockaddr *) &cli_addr, &cli_addr_len);
		if(cli_sock < 0){
			continue;
		}
		
		while(true){
			server_recv_message(buffer, 512)
			
			// Catch disconnect signal
			if(strcmp(buffer, "$DISCONNECT") == 0){
				break;
			}
			
			printf("CLIENT: %s\n",buffer);
		
			server_send_message(reply, 19)
		}
	}
	
	close(cli_sock);
	
	return true;
}*/

int server_send_message(char *message_, size_t length_){
	if(!serv_init){ return false; }

	/*if(write(serv_sock, message_, length_) < 0){
		return false; 
	}*/

	return sendto(serv_sock, message_, length_, 0, (struct sockaddr *) &serv_sa, serv_size);
}

int server_recv_message(char *message_ , size_t length_){
	if(!serv_init){ return false; }

	// Clear the message
	bzero(message_, length_);

	/*if(read(serv_sock, message_, length_) < 0){
		return false;
	}*/
	
	return recvfrom(serv_sock, message_, length_, 0, (struct sockaddr *) &serv_sa, &serv_size);
}

bool close_server(){
	if(!serv_init){ return false; }
	
	close(serv_sock);
	return true;
}
