///@file LinkDef.hpp
///@brief LinkDef for Various ROOT output structures
///@authors T.T. King
///@date 2/6/2018

#ifndef PAASS_LINKDEF_HPP
#define PAASS_LINKDEF_HPP
#ifdef __CINT__
#pragma link C++ struct GAMMASCINT+;
#pragma link C++ class std::vector<GAMMASCINT>+;

#pragma link C++ struct VANDLES+;
#pragma link C++ class std::vector<VANDLES>+;

#pragma link C++ struct CLOVERS+;
#pragma link C++ class std::vector<CLOVERS>+;

#endif

#endif //PAASS_LINKDEF_HPP
