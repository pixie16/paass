/********************************************************************/
/*	csr_test.cpp						    */
/*		last updated: 12/15/09 DTM                          */
/********************************************************************/

#include <iomanip>
#include <iostream>
#include <string>

#include <cstdlib>

#include "pixie16app_defs.h"
#include "pixie16app_export.h"

using namespace std;

int main(int argc, char *argv[])
{
  const size_t num_bits = 19;

  long  cmd1 = 0;

  string CSR_TXT[num_bits];

#if defined(PIF_REVA)
  CSR_TXT[0] = "Respond to group triggers only";
  CSR_TXT[1] = "Measure individual live time";
  CSR_TXT[3] = "Read always";
  CSR_TXT[4] = "Enable trigger";
  CSR_TXT[6] = "GFLT";
#elif defined(PIF_REVF)
  CSR_TXT[CCSRA_TRACEENA] = "Enable trace capture";
  CSR_TXT[CCSRA_QDCENA]   = "Enable QDC sums capture";
  CSR_TXT[10] = "Enable CFD trigger mode";
  CSR_TXT[11] = "Enable global trigger validation";
  CSR_TXT[12] = "Enable raw energy sums capture";
  CSR_TXT[13] = "Enable channel trigger validation";
  CSR_TXT[15] = "Pileup rejection control";
  CSR_TXT[16] = "Hybrid bit";
  CSR_TXT[18] = "SHE single trace capture";
#endif // (else) Rev.A

  CSR_TXT[CCSRA_GOOD]     = "Good Channel";
  CSR_TXT[CCSRA_POLARITY] = "Trigger positive";
  CSR_TXT[CCSRA_ENARELAY] = "HI/LO gain";

  if (argv[1] != NULL)  
    cmd1 = atoi(argv[1]);

  cout << "input " << dec << cmd1 
       << "  HEX 0x" << hex << cmd1 << dec << endl;
  cout << endl << "CSRA bits: " << endl;

  size_t max_len = 0;
  for (size_t k=0; k < num_bits; k++) { 
    if ( CSR_TXT[k].length() > max_len )
      max_len = CSR_TXT[k].length();
  }

  for (size_t k=0; k < num_bits; k++) {
    int retval=APP32_TstBit(k,cmd1);

    cout << retval << " " << setw(max_len) << CSR_TXT[k] << "  "
	 << setw(2) << k << "  " << (1 << k) * retval << endl;
  }

  exit(EXIT_SUCCESS);
}  

