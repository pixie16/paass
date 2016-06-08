/** \file listener.cpp
  * 
  * \brief Receives and decodes packets broadcast by poll2
  * 
  * \author Cory R. Thornsberry
  * 
  * \date April 20th, 2015
  * 
  * \version 1.0
*/

#include <iostream>
#include <string.h>
#include <chrono>
#include <cmath>

#include "poll2_socket.h"

typedef std::chrono::high_resolution_clock hr_clock;
typedef std::chrono::high_resolution_clock::time_point hr_time;

/// Find the order of magnitude of an input double
int order(double input_){
	if(input_ < 0.0){ input_ *= -1; }
	if(input_ >= 1.0){
		for(int i = 0; i <= 100; i++){
			if(input_/std::pow(10.0, (double)i) < 1.0){
				return i-1;
			}
		}
	}
	else{
		for(int i = -100; i <= 1; i++){
			if(input_/std::pow(10.0, (double)i) < 1.0){
				return i-1;
			}
		}
	}
	return 999;
}

int main(){
	char buffer[1024];
	Server poll_server;
	
	size_t total_size;
	int spillID, buffSize;
	int end_packet_flag;
	std::streampos file_size;
	std::streampos new_size;
	
	hr_time clock1;
	hr_time clock2;
	double dT;
	std::chrono::duration<double> time_span;
	
	size_t size_of_int; // The first byte is always the size of an integer on the sending machine
	size_t size_of_spos; // The second byte is always the size of a std::streampos type on the sending machine
	
	std::cout << " Size of integer on this machine: " << sizeof(int) << " bytes\n";
	std::cout << " Size of streampos on this machine: " << sizeof(std::streampos) << " bytes\n";
	
	size_t min_pack_size = 2 + 2*sizeof(int);
	std::cout << " Minimum packet size: " << min_pack_size << " bytes\n\n";
	
	bool first_packet = true;
	if(poll_server.Init(5555)){
		while(true){
			int recv_bytes = poll_server.RecvMessage(buffer, 1024);

			if(strcmp(buffer, "$CLOSE_FILE") == 0){
				std::cout << "  Received CLOSE_FILE flag...\n\n";
				first_packet = true;
				continue;
			}
			else if(strcmp(buffer, "$OPEN_FILE") == 0){
				std::cout << "  Received OPEN_FILE flag...\n\n";
				first_packet = true;
				continue;
			}
			else if(strcmp(buffer, "$KILL_SOCKET") == 0){
				std::cout << "  Received KILL_SOCKET flag...\n\n";
				break;
			}

			// decode the packet
			if(first_packet){
				size_of_int = (size_t)buffer[0];
				size_of_spos = (size_t)buffer[1];
	
				if(size_of_int != sizeof(int) || size_of_spos != sizeof(std::streampos)){ 
					std::cout << "  Warning! basic type size on remote machine does not match local size\n"; 
					std::cout << "  Size of integer on remote machine: " << size_of_int << " bytes\n";
					std::cout << "  Size of streampos on remote machine: " << size_of_spos << " bytes\n\n";
					break;
				}
			}
			else{
				clock2 = hr_clock::now();
				time_span = std::chrono::duration_cast<std::chrono::duration<double> >(clock2 - clock1); // Time between packets in seconds
				if(time_span.count() < 2.0){ continue; }
			}
		
			// The third byte is the start of an integer specifying the total length of the packet
			// (in bytes) including itself and the first two size bytes.
			memcpy((char *)&total_size, &buffer[2], size_of_int);
		
			if(total_size <= min_pack_size){
				// Below is the buffer packet structure
				// ------------------------------------
				// 1 byte size of integer (may not be the same on a different machine)
				// 1 byte size of streampos (may not be the same on a different machine)
				// 4 byte packet length (inclusive, also includes the end packet flag)
				// 4 byte begin packet flag (0xFFFFFFFF)
				std::cout << "  Null packet...\n\n";
			}
			else{
				// Below is the buffer packet structure
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
				char *fname = new char[fname_size+1];
	
				unsigned int index = 2  + size_of_int;
				memcpy(fname, (char *)&buffer[index], fname_size); index += fname_size; // Copy the file name
				memcpy((char *)&new_size, (char *)&buffer[index], size_of_spos); index += size_of_spos; // Copy the file size
				memcpy((char *)&spillID, (char *)&buffer[index], size_of_int); index += size_of_int; // Copy the spill ID
				memcpy((char *)&buffSize, (char *)&buffer[index], size_of_int); index += size_of_int; // Copy the buffer size
				memcpy((char *)&end_packet_flag, (char *)&buffer[index], size_of_int); // Copy the end packet flag
				fname[fname_size] = '\0'; // Terminate the filename string
				
				system("clear");

				std::cout << " recv: " << recv_bytes << " bytes\n";
				std::cout << "  Packet length: " << total_size << " bytes\n";
				std::cout << "  Poll2 filename: " << fname << "\n";
				
				int magnitude = order(new_size);
				if(magnitude < 3){ std::cout << "  Total file size: " << new_size << " B\n"; } // B
				else if(magnitude >= 3 && magnitude < 6){ std::cout << "  Total file size: " << new_size/1E3 << " kB\n"; } // kB
				else if(magnitude >= 6 && magnitude < 9){ std::cout << "  Total file size: " << new_size/1E6 << " MB\n"; } // MB
				else{ std::cout << "  Total file size: " << new_size/1E9 << " GB\n"; } // GB
				
				std::cout << "  Spill number ID: " << spillID << "\n";
				std::cout << "  Buffer size: " << buffSize << " words\n";
				std::cout << "  End packet: " << end_packet_flag << "\n";
				
				if(!first_packet){ 
					dT = time_span.count();
					double rate = ((double)(new_size - file_size))/dT;
					
					magnitude = order(dT);
					if(magnitude > -3){ std::cout << "  Time diff: " << dT << " s\n"; } // s
					else if(magnitude <= -3 && magnitude > -6){ std::cout << "  Time diff: " << dT/1E-3 << " ms\n"; } // ms
					else if(magnitude <= -6 && magnitude > -9){ std::cout << "  Time diff: " << dT/1E-6 << " us\n"; } // us
					else{ std::cout << "  Time diff: " << dT/1E-9 << " ns\n"; } // ns
					
					magnitude = order(rate);
					if(magnitude < 3){ std::cout << "  Data rate: " << rate << " B/s\n"; } // B/s
					else if(magnitude >= 3 && magnitude < 6){ std::cout << "  Data rate: " << rate/1E3 << " kB/s\n"; } // kB/s
					else if(magnitude >= 6 && magnitude < 9){ std::cout << "  Data rate: " << rate/1E6 << " MB/s\n"; } // MB/s
					else{ std::cout << "  Data rate: " << rate/1E9 << " GB/s\n"; } // GB/s
				}
				else{ first_packet = false; }
				std::cout << std::endl;
				
				file_size = new_size;
				delete[] fname;
			}
			clock1 = std::chrono::high_resolution_clock::now();
		}
	}
	else{ return 1; }
	poll_server.Close();

	return 0;
}
