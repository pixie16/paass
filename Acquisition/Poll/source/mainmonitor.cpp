/** \file mainmonitor.cpp
 *
 * \brief Receives and decodes rate packets from StatsHandler, and rebroadcasts packets to the submonitors
 *
 * \author T.T. King, Cory R. Thornsberry, K. Smith, and S. Burcher
 *
 * \date Feb 19, 2023
 *
 */

#include "mainmonitor.hpp"
#include <__config>
using namespace std;

// TODO test what happens when subs are ctrlC'd first then try to quit main
// TODO also figureout best way to handle reading in the Max_Num_submontor var
//  Define the function to be called when ctrl-c (SIGINT) is sent to process
void signal_callback_handler(int signum) {
    cout << "\n    SIGINT:: Sending KILL_SOCKET to the submonitors\n\n"
         << endl;
    if (!DumMode) {
        for (int i = 0; i < 5; ++i) {
            Submonitor_Client_List->at(i).SendMessage((char *)"$KILL_SOCKET", 13);  // Terminate program
        }
    }
    exit(signum);
}

mainmonitor::mainmonitor() : monitor("mainmonitor") {
    poll_server = new Server;
    Submonitor_Client_List = &Submonitor_Clients;
}
mainmonitor::~mainmonitor() {
    if (this->GetDummyMode() && poll_server) {
        delete[] poll_server;
    }
}
void mainmonitor::CloseSubmonitors() {
    if (!GetDummyMode()) {
        for (int i = 0; i < GetMaxNumSubMonitors(); ++i) {
            GetSubClientVec()->at(i).Close();
        }
    }
}
void mainmonitor::RelayPoll2msg(char *buffer, size_t msg_size) {
    for (int i = 0; i < MAX_NUM_SUBMONITORS; ++i) {
        Submonitor_Clients.at(i).SendMessage(buffer, msg_size);
    }
}

void mainmonitor::OpenSubmonitorSockets() {
    std::cout << " Opening relay ports for submonitors" << std::endl;
    std::string submonitorPortMessage = " Opened Client on ports ";
    for (int i = 0; i < MAX_NUM_SUBMONITORS; ++i) {
        Submonitor_Clients.emplace_back(Client());
        int port2open = PREDEFINED_POLL2_PORT + i + 1;
        if (Submonitor_Clients.at(i).Init("127.0.0.1", port2open)) {
            submonitorPortMessage = submonitorPortMessage + std::to_string(port2open);
            if (i == MAX_NUM_SUBMONITORS - 1) {
                submonitorPortMessage += ".";
            } else {
                submonitorPortMessage += ", ";
            }
        } else {
            std::cout << "FAILED to open Client on port " << port2open << std::endl;
        }
    }
    std::cout << submonitorPortMessage.c_str() << std::endl;
}

int main(int argc, char *argv[]) {
    mainmonitor mmon;

    int parRetVal = mmon.ParseCliFlags(argc, argv, &mmon);
    if (parRetVal != 0) {
        return parRetVal;
    }

    // needed for sig handler
    DumMode = mmon.GetDummyMode();

    Server *pserv = mmon.GetPollServer();

    char buffer[mmon.GetPoll2MsgSize()];
    size_t msg_size = mmon.GetPoll2MsgSize();

    monitor::colorThresholds colThreshStruct;
    mmon.SetColorThresholdStruct(colThreshStruct, mmon.GetColorThreshGroup());

    signal(SIGINT, signal_callback_handler);

    int num_modules;
    bool first_packet = true;

    // These are for the dummy mode. I doesnt seem to affect the normal mode startup at all
    int counter = 0;
    time_t curtime = time(NULL);
    srand((unsigned)curtime);

    monitor::poll2_UDP_msg pUdpMsg;

    if (pserv->Init(mmon.GetPredefinedPoll2Port()) || mmon.GetDummyMode()) {
        if (!mmon.GetDummyMode()) {
            mmon.OpenSubmonitorSockets();
        };
        cout << "\n Waiting for first stats packet...\n";

        if (mmon.GetDummyMode()) {
            num_modules = 11;
            for (int it = 0; it < 10; ++it) {  // 10 dead chans in dummy mode
                mmon.GetDeadChanList()->emplace_back(make_pair((rand() % num_modules), (rand() % 16)));
            }
        } else {
            mmon.GetDeadChanList()->emplace_back(make_pair(-1, -1));
        }

        while (true) {
            if (counter == 50 && mmon.GetDummyMode()) {
                break;
            }
            cout << setprecision(2);
            if (!mmon.GetDummyMode()) {
                pserv->RecvMessage(buffer, msg_size);
                char *ptr = buffer;

                mmon.RelayPoll2msg(buffer, msg_size);

                if (strcmp(ptr, "$KILL_SOCKET") == 0) {
                    cout << "  Received KILL_SOCKET flag...\n\n";
                    break;
                }

                system("clear");

                /*
                cout << " Received:\t" << recv_bytes << " bytes\n";

                Below is the stats packet structure (for N modules)
                ---------------------------------------------------
                4 byte total number of pixie modules (N)
                8 byte total time of run (in seconds)
                8 byte total data rate (in B/s)
                channel 0, 0 rate
                channel 0, 0 total
                channel 0, 1 rate
                channel 0, 1 total
                ...
                channel 0, 15 rate
                channel 0, 15 total
                channel 1, 0 rate
                channel 1, 0 total
                ...
                channel N-1, 15 rate
                channel N-1, 15 total
                */
                memcpy(&num_modules, ptr, 4);
                ptr += 4;

                mmon.DecodeUdpMsg(ptr, pUdpMsg, num_modules, first_packet);
            } else {
                system("clear");
                mmon.DecodeUdpMsg(pUdpMsg, num_modules, first_packet, (*mmon.GetDeadChanList()));
            }

            // Display the rate information
            cout << "Run Time: " << mmon.GetTimeString(pUdpMsg.time_in_sec);
            if (num_modules > 1)
                cout << "\t";
            else
                cout << "\n";
            if (mmon.GetDummyMode()) {
                cout << "Data Rate: " << mmon.GetRateString(pUdpMsg.data_rate, mmon.GetColorOut()) << mmon.GetEscSequence(monitor::FG_RED, mmon.GetColorOut())
                     << " DummyMode= " << mmon.GetDummyMode() << mmon.GetEscSequence(monitor::FG_DEFAULT, mmon.GetColorOut()) << endl;
            } else {
                cout << "Data Rate: " << mmon.GetRateString(pUdpMsg.data_rate, mmon.GetColorOut()) << endl;
            }
            int numberOfModsPerRow = (int)ceil((float)num_modules / (float)mmon.GetNumOfModRows());
            int firstModInRow = 0;
            for (int rowNum = 1; rowNum <= mmon.GetNumOfModRows(); ++rowNum) {
                if (rowNum > 1) {
                    cout << setw(mmon.GetModColumWidth() * numberOfModsPerRow + 9) << setfill('-') << "\n";
                }
                cout << "   ";
                for (int i = firstModInRow; i < (rowNum * numberOfModsPerRow); i++) {
                    if (i == num_modules) {
                        break;
                    }
                    cout << "|"
                         << setw((int)((mmon.GetModColumWidth() - 1. + 0.5) / 2))
                         << setfill('-') << "M" << setw(2)
                         << setfill('0') << i
                         << setw((int)((mmon.GetModColumWidth() - 2. + 0.5) / 2))
                         << setfill('-') << "";
                }
                cout << "|\n";

                cout << "   | ";
                for (int j = firstModInRow; j < (rowNum * numberOfModsPerRow); j++) {
                    if (j == num_modules) {
                        break;
                    }
                    cout << "ICR  ";
                    cout << " OCR ";
                    cout << " Data ";
                    cout << "  Total | ";
                }
                cout << "\n";
                pair<string, monitor::ColorCode> ICR;
                pair<string, monitor::ColorCode> OCR;
                pair<string, monitor::ColorCode> DATA;
                pair<string, monitor::ColorCode> TOTALS;
                for (unsigned int i = 0; i < 16; i++) {
                    cout << mmon.GetEscSequence(monitor::FG_DEFAULT, mmon.GetColorOut()) << "C" << setw(2) << setfill('0') << i << "|";
                    for (int j = firstModInRow; j < (rowNum * numberOfModsPerRow); j++) {
                        if (j == num_modules) {
                            break;
                        }
                        ICR = mmon.GetChanRateString(pUdpMsg.ICR[j][i], colThreshStruct);
                        OCR = mmon.GetChanRateString(pUdpMsg.OCR[j][i], colThreshStruct);
                        DATA = mmon.GetChanRateString(pUdpMsg.Data[j][i], colThreshStruct);
                        TOTALS = mmon.GetChanTotalString(pUdpMsg.Totals[j][i]);
                        if (strcmp(TOTALS.first.c_str(), "0") == 0 && strcmp(DATA.first.c_str(), "   0 ") == 0) {
                            ICR.second = mmon.GetColorFromThresholds(-999, colThreshStruct);
                            OCR.second = mmon.GetColorFromThresholds(-999, colThreshStruct);
                            DATA.second = mmon.GetColorFromThresholds(-999, colThreshStruct);
                            TOTALS.second = mmon.GetColorFromThresholds(-999, colThreshStruct);
                        }

                        cout << mmon.GetEscSequence(ICR.second, mmon.GetColorOut()) << setw(5) << setfill(' ')
                             << ICR.first << " " << mmon.GetEscSequence(monitor::FG_DEFAULT, mmon.GetColorOut());
                        cout << mmon.GetEscSequence(OCR.second, mmon.GetColorOut()) << setw(5) << setfill(' ')
                             << OCR.first << " " << mmon.GetEscSequence(monitor::FG_DEFAULT, mmon.GetColorOut());
                        cout << mmon.GetEscSequence(DATA.second, mmon.GetColorOut()) << setw(5) << setfill(' ')
                             << DATA.first << " " << mmon.GetEscSequence(monitor::FG_DEFAULT, mmon.GetColorOut());
                        cout << setw(6) << TOTALS.first << " ";
                        cout << "|";
                    }
                    cout << "\n";
                }
                firstModInRow = firstModInRow + numberOfModsPerRow;
            }
            if (mmon.GetDummyMode()) {
                sleep(2);
                counter++;
            }
        }
    } else {
        cout << " Error: Failed to open poll socket 5556! mainmonitor is probably running. Use the submonitor program.\n";
        return 1;
    }
    if (!mmon.GetDummyMode() && mmon.GetPollServer()) {
        pserv->Close();
    }

    if (pUdpMsg.ICR) {
        delete[] pUdpMsg.ICR;
    }
    if (pUdpMsg.OCR) {
        delete[] pUdpMsg.OCR;
    }
    if (pUdpMsg.Data) {
        delete[] pUdpMsg.Data;
    }
    if (pUdpMsg.Totals) {
        delete[] pUdpMsg.Totals;
    }
    if (Submonitor_Client_List && !mmon.GetDummyMode()) {
        mmon.CloseSubmonitors();
        Submonitor_Client_List = nullptr;
    }
    return 0;
}
