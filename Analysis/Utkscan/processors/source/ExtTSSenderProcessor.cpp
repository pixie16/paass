#include "ChanEvent.hpp"
#include "ExtTSSenderProcessor.hpp"

ExtTSSenderProcessor::ExtTSSenderProcessor() : EventProcessor(7998, 1, "ExtTSSenderProcessor"),
slotNum_(0),
chanNum_(0),
port_(12345),
hostName_("localhost"),
buffSize_(64),
curPos_(0)
{
    Init(hostName_, port_);
}

ExtTSSenderProcessor::ExtTSSenderProcessor(const std::string &type, const std::string &hostName,
                                           const int &slot, const int &channel, const int &port, const int &buffSize) :
EventProcessor(7998, 1, "ExtTSSenderProcessor")
{
    type_ = type;
    slotNum_ = slot;
    chanNum_ = channel;
    buffSize_ = buffSize;
    Init(hostName, port);
    associatedTypes.insert(type_);
}

ExtTSSenderProcessor::~ExtTSSenderProcessor()
{
    SendTS();
    Close();
    if(udpClient_) delete udpClient_;
    if(buffer_) delete buffer_;
}

bool ExtTSSenderProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;

    static const std::vector<ChanEvent *> &chEvents = event.GetEventList();
    for( auto chEvent : chEvents ) {
        if( chEvent->GetChannelNumber() == chanNum_ && chEvent->GetModuleNumber() == slotNum_ ){
            unsigned long long ts = chEvent->GetExternalTimeStamp();
            // printf("ts %llu \n", ts);
            SetTS(ts);
        }
    }

    EndProcess();
    return true;
}

void ExtTSSenderProcessor::SetBuffSize(const unsigned int &buffSize)
{
    if(buffer_)
      delete buffer_;
    buffer_ = new unsigned long long int[buffSize];

    ClearBuff();

    buffSize_ = buffSize;

}

bool ExtTSSenderProcessor::Init(const std::string &hostName, const int &port)
{
    hostName_ = hostName;
    port_ = port;

    udpClient_ = new Client();
    SetBuffSize(buffSize_);

    return udpClient_->Init(hostName_.c_str(),port_);
}

int ExtTSSenderProcessor::SendTS()
{
    return udpClient_->SendMessage((char*)buffer_,8*buffSize_);
}

void ExtTSSenderProcessor::SetTS(const unsigned long long int &ts)
{
    buffer_[curPos_] = ts;
    curPos_++;

    if(curPos_ >= buffSize_){
      SendTS();
      ClearBuff();
    }

}

void ExtTSSenderProcessor::ClearBuff()
{
    for(unsigned int i=0; i<buffSize_; i++){
      buffer_[i] = 0;
    }
    curPos_ = 0;
}
