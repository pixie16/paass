/** \file monitor.cpp
  * 
  * \brief Receives and decodes rate packets from StatsHandler
  * 
  * \author Cory R. Thornsberry
  * 
  * \date June 4th, 2015
  * 
  * \version 1.0
*/

#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "poll2_socket.h"

#define KILOBYTE 1024 // bytes
#define MEGABYTE 1048576 // bytes
#define GIGABYTE 1073741824 // bytes

// Return the order of magnitude of a number
double GetOrder(unsigned int input_, unsigned int &power){
	double test = 1;
	for(unsigned int i = 0; i < 100; i++){
		if(input_/test <= 1){ 
			power = i;
			return test; 
		}
		test *= 10.0;
	}
	return 1;
}

// Expects input rate in Hz
std::string GetChanRateString(double input_){
	if(input_ < 0.0){ input_ *= -1; }
	
	std::stringstream stream;
	if(input_/1E6 > 1){ stream << input_/1E6 << "M"; } // MHz
	else if(input_/1E3 > 1){ stream << input_/1E3 << "k"; } // kHz
	else{ stream << input_; } // Hz
	
	// Limit to 1 decimal place due to space constraints
	std::string output = stream.str();
	size_t find_index = output.find('.');
	if(find_index != std::string::npos){
		std::string temp;
		temp = output.substr(0, find_index);
		temp += output.substr(find_index, 2);
		output = temp;
	}
	
	if(input_/1E6 > 1){ output += "M"; } // MHz
	else if(input_/1E3 > 1){ output += "k"; } // kHz
	
	return output;
}

std::string GetChanTotalString(unsigned int input_){
	std::stringstream stream;
	unsigned int power = 0;
	double order = GetOrder(input_, power);
	
	if(power >= 4){ stream << 10*input_/order; }
	else{ stream << input_; }
	
	// Limit to 2 decimal place due to space constraints
	std::string output = stream.str();
	size_t find_index = output.find('.');
	if(find_index != std::string::npos){
		std::string temp;
		temp = output.substr(0, find_index);
		temp += output.substr(find_index, 3);
		output = temp;
	}

	if(power >= 4){ 
		std::stringstream stream2;
		stream2 << output << "E" << power-1; 
		output = stream2.str();
	}

	return output;
}

// Expects input rate in B/s
std::string GetRateString(double input_){
	if(input_ < 0.0){ input_ *= -1; }
	
	std::stringstream stream;
	if(input_/GIGABYTE > 1){ stream << input_/GIGABYTE << " GB/s\n"; } // GB/s
	else if(input_/MEGABYTE > 1){ stream << input_/MEGABYTE << " MB/s\n"; } // MB/s
	else if(input_/KILOBYTE > 1){ stream << input_/KILOBYTE << " kB/s\n"; } // kB/s
	else{ stream << input_ << " B/s\n"; } // B/s
	
	return stream.str();
}

// Expects input time in seconds
std::string GetTimeString(double input_){
	if(input_ < 0.0){ input_ *= -1; }
	
	long long time = (long long)input_;
	int hr = time/3600;
	int min = (time%3600)/60;
	int sec = (time%3600)%60;
	int rem = (int)(100*(input_ - time));
	
	std::stringstream stream; 
	if(hr < 10){ stream << "0" << hr; }
	else{ stream << hr; }
	stream << ":";
	if(min < 10){ stream << "0" << min; }
	else{ stream << min; }
	stream << ":";
	if(sec < 10){ stream << "0" << sec; }
	else{ stream << sec; }
	stream << ".";
	if(rem < 10){ stream << "0" << rem; }
	else{ stream << rem; }
	
	return stream.str();
}

int main(){
	char buffer[2048]; // 2 kB of stats data max
	Server poll_server;
	
	int num_modules;
	double time_in_sec;
	double data_rate;
	double **rates = NULL;
	unsigned int **totals = NULL;
	
	bool first_packet = true;
	if(poll_server.Init(5556)){
		while(true){
			std::cout << std::setprecision(2);
		
			int recv_bytes = poll_server.RecvMessage(buffer, 2048);
			char *ptr = buffer;

			if(strcmp(buffer, "$KILL_SOCKET") == 0){
				std::cout << "  Received KILL_SOCKET flag...\n\n";
				break;
			}

			system("clear");
			
			std::cout << " Received: " << recv_bytes << " bytes\n";
		
			// Below is the stats packet structure (for N modules)
			// ---------------------------------------------------
			// 4 byte total number of pixie modules (N)
			// 8 byte total time of run (in seconds)
			// 8 byte total data rate (in B/s)
			// channel 0, 0 rate
			// channel 0, 0 total
			// channel 0, 1 rate
			// channel 0, 1 total
			// ...
			// channel 0, 15 rate
			// channel 0, 15 total
			// channel 1, 0 rate
			// channel 1, 0 total
			// ...
			// channel N-1, 15 rate
			// channel N-1, 15 total
			memcpy(&num_modules, ptr, 4); ptr += 4;

			if(first_packet){
				rates = new double*[num_modules];
				totals = new unsigned int*[num_modules];
				for(int i = 0; i < num_modules; i++){
					rates[i] = new double[16];
					totals[i] = new unsigned int[16];
				}
				first_packet = false;
			}
			
			memcpy(&time_in_sec, ptr, 8); ptr += 8;
			memcpy(&data_rate, ptr, 8); ptr += 8;
			for(int i = 0; i < num_modules; i++){
				for(int j = 0; j < 16; j++){
					memcpy(&rates[i][j], ptr, 8); ptr += 8;
					memcpy(&totals[i][j], ptr, 4); ptr += 4;
				}
			}
			
			// Display the rate information
			std::cout << " Spill Time: " << GetTimeString(time_in_sec) << std::endl;
			std::cout << " Data Rate: " << GetRateString(data_rate) << std::endl;
			std::cout << "        |------M00------";
			for(unsigned int i = 1; i < (unsigned int)num_modules; i++){
				if(i < 10){ std::cout << "|------M0" << i << "------"; }
				else{ std::cout << "|------M" << i << "------"; }
			}
			std::cout << "|\n";
				
			for(unsigned int i = 0; i < 16; i++){
				if(i < 10){ std::cout << "  C0" << i; }
				else{ std::cout << "  C" << i; }
				for(unsigned int j = 0; j < (unsigned int)num_modules; j++){
					std::cout << "\t|" << GetChanRateString(rates[j][i]) << "\t" << GetChanTotalString(totals[j][i]);
				}
				std::cout << "|\n";
			}
			std::cout << "\n";
		}
	}
	else{ 
		std::cout << " Error: Failed to open poll socket 5556!\n";
		return 1; 
	}
	poll_server.Close();
	
	if(rates){ delete[] rates; }
	if(totals){ delete[] totals; }

	return 0;
}
