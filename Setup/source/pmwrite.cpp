/********************************************************************/
/*	pmwrite.cpp                                                       */
/*		last updated: April 19th, 2015 CRT                          */
/********************************************************************/

#include <iostream>

#include "PixieSupport.h"

int main(int argc, char *argv[])
{
	if(argc < 4){
		std::cout << " Invalid number of arguments to " << argv[0] << std::endl;
		std::cout << "  SYNTAX: " << argv[0] << " [module] [parameter] [value]\n\n";
		return 1;
	}

	int mod = atoi(argv[1]);
	unsigned long value = (unsigned long)atol(argv[3]);

	PixieInterface pif("pixie.cfg");

	pif.GetSlots();
	pif.Init();
	pif.Boot(PixieInterface::DownloadParameters | PixieInterface::ProgramFPGA | PixieInterface::SetDAC, true);

	std::string temp_str(argv[2]);
	ParameterModuleWriter writer;
	if(forModule(&pif, mod, writer, make_pair(temp_str, value))){ pif.SaveDSPParameters(); }

	return 0;
}
