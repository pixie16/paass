/** \file listener.cpp
  * 
  * \brief Receives and decodes packets broadcast by poll2
  * 
  * \author Cory R. Thornsberry
  * 
  * \date April 20th, 2015
*/

#include <iostream>

#include "poll2_socket.h"

int main(){
	char buffer[1024];
	Server poll_server;
	
	int total_size, spillID, buffSize;
	int end_packet_flag;
	std::streampos file_size;
	
	size_t size_of_int; // The first byte is always the size of an integer on the sending machine
	size_t size_of_spos; // The second byte is always the size of a std::streampos type on the sending machine
	
	std::cout << " Size of integer on this machine: " << sizeof(int) << " bytes\n";
	std::cout << " Size of streampos on this machine: " << sizeof(std::streampos) << " bytes\n";
	
	size_t min_pack_size = 2 + 2*sizeof(int);
	std::cout << " Minimum packet size: " << min_pack_size << " bytes\n\n";
	
	bool first_packet = true;
	if(poll_server.Init("127.0.0.1", 5555)){
		while(true){
			std::cout << " recv: " << poll_server.RecvMessage(buffer, 1024) << " bytes\n";
		
			// decode the packet
			if(first_packet){
				size_of_int = (size_t)buffer[0];
				size_of_spos = (size_t)buffer[1];
		
				if(size_of_int != sizeof(int)){ 
					std::cout << "  Warning! size of remote integer does not match this local machine\n"; 
					break;
				}
				if(size_of_spos != sizeof(std::streampos)){ 
					std::cout << "  Warning! size of remote streampos does not match this local machine\n"; 
					break;
				}
				
				first_packet = false;
			}
		
			// The third byte is the start of an integer specifying the total length of the packet
			// (in bytes) including itself and the first two size bytes.
			memcpy(&total_size, buffer[2], size_of_int);
		
			if(total_size <= min_pack_size){
				// Below is the output packet structure
				// ------------------------------------
				// 1 byte size of integer (may not be the same on a different machine)
				// 1 byte size of streampos (may not be the same on a different machine)
				// 4 byte packet length (inclusive, also includes the end packet flag)
				// 4 byte begin packet flag (0xFFFFFFFF)
				std::cout << "  Null packet...\n";
			}
			else{
				// Below is the output packet structure
				// ------------------------------------
				// 1 byte size of integer (may not be the same on a different machine)
				// 1 byte size of streampos (may not be the same on a different machine)
				// 4 byte packet length (inclusive, also includes the end packet flag)
				// x byte file path (no size limit)
				// 8 byte file size streampos (long long)
				// 4 byte spill number ID (unsigned int)
				// 4 byte buffer size (unsigned int)
				// 4 byte end packet flag (0xFFFFFFFF)
				size_t fname_size = total_size - (2 + 4*size_of_int) - size_of_spos; // Size of the filename (in bytes)
				char fname[fname_size+1];
	
				unsigned int index = 2  + size_of_int;
				memcpy(fname, (char *)&output[index], 1); index += fname_size; // Copy the file name
				memcpy((char *)&file_size, (char *)&output[index], size_of_spos); index += size_of_spos; // Copy the file size
				memcpy((char *)&spillID, (char *)&output[index], size_of_int); index += size_of_int; // Copy the spill ID
				memcpy((char *)&buffSize, (char *)&output[index], size_of_int); index += size_of_int; // Copy the buffer size
				memcpy((char *)&end_packet_flag, (char *)&output[index], size_of_int); // Copy the end packet flag
				fname[fname_size] = '\0'; // Terminate the filename string
			}
		}
	}
	else{ return 1; }
	poll_server.Close();

	return 0;
}
