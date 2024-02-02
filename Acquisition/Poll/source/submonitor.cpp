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

submonitor::submonitor() : monitor("submonitor") {
    this->poll_server = new Server();
}
submonitor::~submonitor() = default;

int main(int argc, char *argv[]) {
    submonitor smon;
    int parRetVal = smon.ParseCliFlags(argc, argv, &smon);
    if (parRetVal != 0) {
        return parRetVal;
    } else if (parRetVal == 0 && smon.GetSocketToUse() != 0) {
        smon.isInit = true;
    } else {
        cout << endl
             << endl
             << smon.GetEscSequence(monitor::FG_RED, true) << "Argument Error:: \"-s\" flag (socket) is required for `submonitor` operation" << smon.GetEscSequence(monitor::FG_DEFAULT, true) << endl;
        smon.help(smon.GetName());
        return 1;
    }

    DumMode = smon.GetDummyMode();

    Server *pserv = smon.GetPollServer();

    char buffer[smon.GetPoll2MsgSize()];
    size_t msg_size = smon.GetPoll2MsgSize();

    monitor::colorThresholds colThreshStruct;
    smon.SetColorThresholdStruct(colThreshStruct, smon.GetColorThreshGroup());

    int num_modules;
    bool first_packet = true;

    monitor::poll2_UDP_msg pUdpMsg;

    if (pserv->Init(smon.GetSocketToUse())) {
        cout << " Waiting for first stats packet on port " << smon.GetSocketToUse() << " ...\n";

        while (true) {
            cout << std::setprecision(2);
            pserv->RecvMessage(buffer, msg_size);
            char *ptr = buffer;

            if (strcmp(buffer, "$KILL_SOCKET") == 0) {
                cout << "  Received KILL_SOCKET flag from mainmonitor...\n\n";
                break;
            }

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

            smon.DecodeUdpMsg(ptr, pUdpMsg, num_modules, first_packet);
            // Display the rate information
            cout << "Run Time: " << smon.GetTimeString(pUdpMsg.time_in_sec);
            if (num_modules > 1)
                cout << "\t";
            else
                cout << "\n";

            cout << "Data Rate: " << smon.GetRateString(pUdpMsg.data_rate, smon.GetColorOut()) << endl;

            int numberOfModsPerRow = (int)ceil((float)num_modules / (float)smon.GetNumOfModRows());
            int firstModInRow = 0;
            for (int rowNum = 1; rowNum <= smon.GetNumOfModRows(); ++rowNum) {
                if (rowNum > 1) {
                    cout << setw(smon.GetModColumWidth() * numberOfModsPerRow + 9) << setfill('-') << "\n";
                }
                cout << "   ";
                for (int i = firstModInRow; i < (rowNum * numberOfModsPerRow); i++) {
                    if (i == num_modules) {
                        break;
                    }
                    cout << "|"
                         << setw((int)((smon.GetModColumWidth() - 1. + 0.5) / 2))
                         << setfill('-') << "M" << setw(2)
                         << setfill('0') << i
                         << setw((int)((smon.GetModColumWidth() - 2. + 0.5) / 2))
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
                    cout << smon.GetEscSequence(monitor::FG_DEFAULT, smon.GetColorOut()) << "C" << setw(2) << setfill('0') << i << "|";
                    for (int j = firstModInRow; j < (rowNum * numberOfModsPerRow); j++) {
                        if (j == num_modules) {
                            break;
                        }
                        ICR = smon.GetChanRateString(pUdpMsg.ICR[j][i], colThreshStruct);
                        OCR = smon.GetChanRateString(pUdpMsg.OCR[j][i], colThreshStruct);
                        DATA = smon.GetChanRateString(pUdpMsg.Data[j][i], colThreshStruct);
                        TOTALS = smon.GetChanTotalString(pUdpMsg.Totals[j][i]);
                        // printf("T=|%s|",DATA.first.c_str());
                        if (strcmp(TOTALS.first.c_str(), "0") == 0 && strcmp(DATA.first.c_str(), "   0 ") == 0) {
                            ICR.second = smon.GetColorFromThresholds(-999, colThreshStruct);
                            OCR.second = smon.GetColorFromThresholds(-999, colThreshStruct);
                            DATA.second = smon.GetColorFromThresholds(-999, colThreshStruct);
                            TOTALS.second = smon.GetColorFromThresholds(-999, colThreshStruct);
                            // printf("cols=|%i|,|%i|,|%i|,|%i|",ICR.second,OCR.second,DATA.second,TOTALS.second);
                        }

                        cout << smon.GetEscSequence(ICR.second, smon.GetColorOut()) << setw(5) << setfill(' ')
                             << ICR.first << " " << smon.GetEscSequence(monitor::FG_DEFAULT, smon.GetColorOut());
                        cout << smon.GetEscSequence(OCR.second, smon.GetColorOut()) << setw(5) << setfill(' ')
                             << OCR.first << " " << smon.GetEscSequence(monitor::FG_DEFAULT, smon.GetColorOut());
                        cout << smon.GetEscSequence(DATA.second, smon.GetColorOut()) << setw(5) << setfill(' ')
                             << DATA.first << " " << smon.GetEscSequence(monitor::FG_DEFAULT, smon.GetColorOut());
                        cout << setw(6) << TOTALS.first << " ";
                        cout << "|";
                    }
                    cout << "\n";
                }
                firstModInRow = firstModInRow + numberOfModsPerRow;
            }
        }
    } else {
        cout << " Error: Failed to open submonitor socket " << smon.GetSocketToUse() << "!\n";
        return 1;
    }

    if (smon.GetPollServer()) {
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
    return 0;
}
