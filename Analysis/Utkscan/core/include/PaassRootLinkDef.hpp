///@file SysRootLinkDef.hpp
///@brief SysRootLinkDef for Various ROOT output structures
///@authors T.T. King, R. Yokoyama
///@date 2/6/2018

#ifndef PAASS_LINKDEF_HPP
#define PAASS_LINKDEF_HPP
#ifdef __CINT__

#pragma link C++ struct processor_struct::BATO+;
#pragma link C++ class std::vector<processor_struct::BATO>+;

#pragma link C++ struct processor_struct::CLOVER+;
#pragma link C++ class std::vector<processor_struct::CLOVER>+;

#pragma link C++ struct processor_struct::DOUBLEBETA+;
#pragma link C++ class std::vector<processor_struct::DOUBLEBETA>+;

#pragma link C++ struct processor_struct::GAMMASCINT+;
#pragma link C++ class std::vector<processor_struct::GAMMASCINT>+;

#pragma link C++ struct processor_struct::LOGIC+;
#pragma link C++ class std::vector<processor_struct::LOGIC>+;

#pragma link C++ struct processor_struct::MTAS+;
#pragma link C++ class std::vector<processor_struct::MTAS>+;

#pragma link C++ struct processor_struct::NEXT+;
#pragma link C++ class std::vector<processor_struct::NEXT>+;

#pragma link C++ struct processor_struct::PID+;
#pragma link C++ class std::vector<processor_struct::PID>+;

#pragma link C++ struct processor_struct::PSPMT+;
#pragma link C++ class std::vector<processor_struct::PSPMT>+;

#pragma link C++ struct processor_struct::ROOTDEV+;
#pragma link C++ class std::vector<processor_struct::ROOTDEV>+;

#pragma link C++ struct processor_struct::SINGLEBETA+;
#pragma link C++ class std::vector<processor_struct::SINGLEBETA>+;

#pragma link C++ struct processor_struct::VANDLE+;
#pragma link C++ class std::vector<processor_struct::VANDLE>+;

#pragma link C++ class PixTreeEvent+;
#pragma link C++ class std::vector<PixTreeEvent>+;
#endif

#endif //PAASS_LINKDEF_HPP
