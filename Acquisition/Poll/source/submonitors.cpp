/** \file submonitor.cpp
 *
 * \brief Receives and decodes rate packets from mainmonitor
 *
 * \author T.T. King, Cory R. Thornsberry, K. Smith, and S. Burcher
 *
 * \date Feb 19th, 2023
 *
 */

#include "submonitor.hpp"

using namespace std;

// void signal_callback_handler(int signum) {
//     cout << "\n    SIGINT:: Sending KILL_SOCKET to the submonitors\n\n"
//          << endl;
//     poll_server->Close();
//     cout<<"test"<<endl;
//     exit(signum);
// }

submonitor::submonitor() : monitor("submonitor") {
    poll_server = new Server;
}
submonitor::~submonitor() {
    if (poll_server) {
        delete[] poll_server;
    }
    //     delete Submonitor_Client_List;
}

int main(int argc, char *argv[]) {
    submonitor mmon;
    int parRetVal = mmon.ParseCliFlags(argc, argv, &mmon);
    if (parRetVal != 0) {
        return parRetVal;
    }

    DumMode = mmon.GetDummyMode();

    Server *pserv = mmon.GetPollServer();

    char buffer[mmon.GetPoll2MsgSize()];
    size_t msg_size = mmon.GetPoll2MsgSize();

    monitor::colorThresholds colThreshStruct;
    mmon.SetColorThresholdStruct(colThreshStruct, mmon.GetColorThreshGroup());

    int num_modules;
    bool first_packet = true;

    int counter = 0;
    time_t curtime = time(NULL);
    srand((unsigned)curtime);
    monitor::poll2_UDP_msg pUdpMsg;

    if (pserv->Init(mmon.GetSocketToUse())) {
        cout << " Waiting for first stats packet on port " << mmon.GetSocketToUse() << " ...\n";

        while (true) {
            cout << std::setprecision(2);

            pserv->RecvMessage(buffer, msg_size);
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

            mmon.DecodeUdpMsg(ptr, pUdpMsg, num_modules, first_packet);
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
            cout << "   ";
            for (unsigned int i = 0; i < (unsigned int)num_modules; i++) {
                cout << "|"
                     << setw((int)((mmon.GetModColumWidth() - 1. + 0.5) / 2))
                     << setfill('-') << "M" << setw(2)
                     << setfill('0') << i
                     << setw((int)((mmon.GetModColumWidth() - 2. + 0.5) / 2))
                     << setfill('-') << "";
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
            pair<string, monitor::ColorCode> ICR;
            pair<string, monitor::ColorCode> OCR;
            pair<string, monitor::ColorCode> DATA;
            pair<string, monitor::ColorCode> TOTALS;
            for (unsigned int i = 0; i < 16; i++) {
                cout << mmon.GetEscSequence(monitor::FG_DEFAULT, mmon.GetColorOut()) << "C" << setw(2) << setfill('0') << i << "|";
                for (unsigned int j = 0; j < (unsigned int)num_modules; j++) {
                    ICR = mmon.GetChanRateString(pUdpMsg.ICR[j][i], colThreshStruct);
                    OCR = mmon.GetChanRateString(pUdpMsg.OCR[j][i], colThreshStruct);
                    DATA = mmon.GetChanRateString(pUdpMsg.Data[j][i], colThreshStruct);
                    TOTALS = mmon.GetChanTotalString(pUdpMsg.Totals[j][i]);
                    // printf("T=|%s|",DATA.first.c_str());
                    if (strcmp(TOTALS.first.c_str(), "0") == 0 && strcmp(DATA.first.c_str(), "   0 ") == 0) {
                        ICR.second = mmon.GetColorFromThresholds(-999, colThreshStruct);
                        OCR.second = mmon.GetColorFromThresholds(-999, colThreshStruct);
                        DATA.second = mmon.GetColorFromThresholds(-999, colThreshStruct);
                        TOTALS.second = mmon.GetColorFromThresholds(-999, colThreshStruct);
                        // printf("cols=|%i|,|%i|,|%i|,|%i|",ICR.second,OCR.second,DATA.second,TOTALS.second);
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
        }
    } else {
        cout << " Error: Failed to open submonitor socket " << mmon.GetSocketToUse() << "!\n";
        return 1;
    }
    cout<<"ps close()"<< endl;
        pserv->Close();
    cout<<"ps close 2"<<endl;
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
    return 0;
}
