///@file LinkDef.hpp
///@brief LinkDef for Various ROOT output structures
///@authors T.T. King
///@date 2/6/2018

#ifndef PAASS_LINKDEF_HPP
#define PAASS_LINKDEF_HPP
#ifdef __CINT__
#pragma link C++ struct processor_struct::GAMMASCINT+;
#pragma link C++ class std::vector<processor_struct::GAMMASCINT>+;

#pragma link C++ struct processor_struct::VANDLES+;
#pragma link C++ class std::vector<processor_struct::VANDLES>+;

#pragma link C++ struct processor_struct::CLOVERS+;
#pragma link C++ class std::vector<processor_struct::CLOVERS>+;

#pragma link C++ struct processor_struct::PSPMT+;
#pragma link C++ class std::vector<processor_struct::PSPMT>+;

#pragma link C++ class PixTreeEvent+;
#pragma link C++ class std::vector<PixTreeEvent>+;
#endif

#endif //PAASS_LINKDEF_HPP
