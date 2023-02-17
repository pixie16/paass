/** \file monitor.cpp
 *
 * \brief Receives and decodes rate packets from StatsHandler
 *
 * \author Cory R. Thornsberry, K. Smith, and S. Burcher
 *
 * \date June 4th, 2015
 *
 */

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "poll2_socket.h"

#define KILOBYTE 1024        // bytes
#define MEGABYTE 1048576     // bytes
#define GIGABYTE 1073741824  // bytes

using std::cout;
using std::endl;

/* Print help dialogue for command line options. */
void help(const char *progName_) {
    std::cout << "\n SYNTAX: " << progName_ << " [options]\n";
    std::cout << "  --color (-c)          | Use colored output (not implemented yet)\n";
    std::cout << "  --log (-l)            | Write monitor log files (not implemented yet)\n";
    std::cout << "  --socket (-s)         | Submonitor socket to use (Default range is 5557-5561)\n";
    std::cout << "  --rows (-r)           | Number of rows to draw monitor on (not implemented yet)\n";
    std::cout << "  --help (-h)           | Display this help dialogue.\n\n";
}

// Return the order of magnitude of a number
double GetOrder(unsigned int input_, unsigned int &power) {
    double test = 1;
    for (unsigned int i = 0; i < 100; i++) {
        if (input_ / test <= 1) {
            power = i;
            return test;
        }
        test *= 10.0;
    }
    return 1;
}

// Expects input rate in Hz
std::string GetChanRateString(double input_) {
    if (input_ < 0.0) {
        input_ *= -1;
    }
    int power = std::log10(input_);

    std::stringstream stream;
    stream << std::setprecision(2) << std::fixed;
    if (power >= 6) {
        stream << input_ / 1E6 << "M";
    }  // MHz
    else if (power >= 3) {
        stream << input_ / 1E3 << "k";
    }  // kHz
    else if (input_ == 0) {
        stream << "   0 ";
    } else {
        stream << input_ << " ";
    }  // Hz

    std::string output = stream.str();
    output = output.substr(0, output.find_last_not_of(".", 3) + 1) +
             output.substr(output.length() - 1, 1);

    return output;
}

std::string GetChanTotalString(unsigned int input_) {
    std::stringstream stream;
    unsigned int power = 0;
    double order = GetOrder(input_, power);

    if (power >= 4) {
        stream << 10 * input_ / order;
    } else {
        stream << input_;
    }

    // Limit to 2 decimal place due to space constraints
    std::string output = stream.str();
    size_t find_index = output.find('.');
    if (find_index != std::string::npos) {
        std::string temp;
        temp = output.substr(0, find_index);
        temp += output.substr(find_index, 3);
        output = temp;
    }

    if (power >= 4) {
        std::stringstream stream2;
        stream2 << output << "E" << power - 1;
        output = stream2.str();
    }

    return output;
}

// Expects input rate in B/s
std::string GetRateString(double input_) {
    if (input_ < 0.0) {
        input_ *= -1;
    }

    std::stringstream stream;
    if (input_ / GIGABYTE > 1) {
        stream << input_ / GIGABYTE << " GB/s\n";
    }  // GB/s
    else if (input_ / MEGABYTE > 1) {
        stream << input_ / MEGABYTE << " MB/s\n";
    }  // MB/s
    else if (input_ / KILOBYTE > 1) {
        stream << input_ / KILOBYTE << " kB/s\n";
    }  // kB/s
    else {
        stream << input_ << " B/s\n";
    }  // B/s

    return stream.str();
}

// Expects input time in seconds
std::string GetTimeString(double input_) {
    if (input_ < 0.0) {
        input_ *= -1;
    }

    long long time = (long long)input_;
    int hr = time / 3600;
    int min = (time % 3600) / 60;
    int sec = (time % 3600) % 60;
    int rem = (int)(100 * (input_ - time));

    std::stringstream stream;
    if (hr < 10) {
        stream << "0" << hr;
    } else {
        stream << hr;
    }
    stream << ":";
    if (min < 10) {
        stream << "0" << min;
    } else {
        stream << min;
    }
    stream << ":";
    if (sec < 10) {
        stream << "0" << sec;
    } else {
        stream << sec;
    }
    stream << ".";
    if (rem < 10) {
        stream << "0" << rem;
    } else {
        stream << rem;
    }

    return stream.str();
}

int main(int argc, char *argv[]) {
    const size_t msg_size = 5844;  // 5.8 kB of stats data max
    const int modColumnWidth = 25;
    char buffer[msg_size];
    Server cur_submonitor_sock;

    int num_modules;
    double time_in_sec;
    double data_rate;
    double **rates = NULL;
    double **inputCountRate = NULL;
    double **outputCountRate = NULL;
    unsigned int **totals = NULL;

    bool useColor = false;
    bool useLogging = false;
    int socketToUse = 0;
    int numRowsOfMods = 1;
    static const int MAX_NUM_SUBMONITORS = 5;
    static const int PREDEFINED_POLL2_PORT = 5556;

    struct option longOpts[] = {
        {"color", no_argument, NULL, 'c'},
        {"log", no_argument, NULL, 'l'},
        {"socket", required_argument, NULL, 's'},
        {"rows", required_argument, NULL, 'r'},
        {"help", no_argument, NULL, 'h'},
        {"?", no_argument, NULL, 0},
        {NULL, no_argument, NULL, 0}};

    int idx = 0;
    int retval = 0;
    while ((retval = getopt_long(argc, argv, "cls:r:h", longOpts, &idx)) != -1) {
        switch (retval) {
            case 'c':
                cout << " Colorized output not supported yet" << endl;
                return 1;
            case 'l':
                cout << " Logging output not supported yet" << endl;
                return 1;
            case 's':
                if ((atoi(optarg) - PREDEFINED_POLL2_PORT) > 0 && (atoi(optarg) - PREDEFINED_POLL2_PORT) <= MAX_NUM_SUBMONITORS) {
                    socketToUse = atoi(optarg);
                    break;
                } else {
                    cout << " un available socket pick the correct one" << endl;
                    return 1;
                };
            case 'r':
                cout << " Row Splitting not supported yet" << endl;
                return 1;
            case 'h':
                help(argv[0]);
                return 2;
            case '?':
                help(argv[0]);
                return 2;
            default:
                break;
        };
    };

    bool first_packet = true;
    if (cur_submonitor_sock.Init(socketToUse)) {
        
        cout << " Waiting for first stats packet on port " << socketToUse <<" ...\n";

        while (true) {
            cout << std::setprecision(2);

            cur_submonitor_sock.RecvMessage(buffer, msg_size);
            char *ptr = buffer;

            if (strcmp(buffer, "$KILL_SOCKET") == 0) {
                cout << "  Received KILL_SOCKET flag from mainmonitor...\n\n";
                break;
            }

            // if (cur_submonitor_sock.)

            system("clear");

            // cout << " Received:\t" << recv_bytes << " bytes\n";

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
            memcpy(&num_modules, ptr, 4);
            ptr += 4;

            if (first_packet) {
                rates = new double *[num_modules];
                inputCountRate = new double *[num_modules];
                outputCountRate = new double *[num_modules];
                totals = new unsigned int *[num_modules];
                for (int i = 0; i < num_modules; i++) {
                    rates[i] = new double[16];
                    inputCountRate[i] = new double[16];
                    outputCountRate[i] = new double[16];
                    totals[i] = new unsigned int[16];
                }
                first_packet = false;
            }

            memcpy(&time_in_sec, ptr, 8);
            ptr += 8;
            memcpy(&data_rate, ptr, 8);
            ptr += 8;
            for (int i = 0; i < num_modules; i++) {
                for (int j = 0; j < 16; j++) {
                    memcpy(&inputCountRate[i][j], ptr, 8);
                    ptr += 8;
                    memcpy(&outputCountRate[i][j], ptr, 8);
                    ptr += 8;
                    memcpy(&rates[i][j], ptr, 8);
                    ptr += 8;
                    memcpy(&totals[i][j], ptr, 4);
                    ptr += 4;
                }
            }

            // Display the rate information
            cout << "Run Time: " << GetTimeString(time_in_sec);
            if (num_modules > 1)
                cout << "\t";
            else
                cout << "\n";
            cout << "Data Rate: " << GetRateString(data_rate) << std::endl;
            cout << "   ";
            for (unsigned int i = 0; i < (unsigned int)num_modules; i++) {
                cout << "|"
                     << std::setw((int)((modColumnWidth - 1. + 0.5) / 2))
                     << std::setfill('-') << "M" << std::setw(2)
                     << std::setfill('0') << i
                     << std::setw((int)((modColumnWidth - 2. + 0.5) / 2))
                     << std::setfill('-') << "";
            }
            cout << "|\n";

            cout << "   | ";
            for (unsigned int j = 0; j < (unsigned int)num_modules; j++) {
                cout << "ICR  ";
                cout << " OCR ";
                cout << " Data ";
                cout << "  Total | ";
            }
            cout << "\n";

            for (unsigned int i = 0; i < 16; i++) {
                cout << "C" << std::setw(2) << std::setfill('0') << i
                     << "|";
                for (unsigned int j = 0; j < (unsigned int)num_modules; j++) {
                    cout << std::setw(5) << std::setfill(' ')
                         << GetChanRateString(inputCountRate[j][i]) << " ";
                    cout << std::setw(5) << std::setfill(' ')
                         << GetChanRateString(outputCountRate[j][i])
                         << " ";
                    cout << std::setw(5) << std::setfill(' ')
                         << GetChanRateString(rates[j][i]) << " ";
                    cout << std::setw(6)
                         << GetChanTotalString(totals[j][i]) << " ";
                    cout << "|";
                }
                cout << "\n";
            }
        }
    } else {
        cout << " Error: Failed to open submonitor socket " << socketToUse << "!\n";
        return 1;
    }
    cur_submonitor_sock.Close();

    if (rates) {
        delete[] rates;
    }
    if (inputCountRate) {
        delete[] inputCountRate;
    }
    if (outputCountRate) {
        delete[] outputCountRate;
    }
    if (totals) {
        delete[] totals;
    }

    return 0;
}
