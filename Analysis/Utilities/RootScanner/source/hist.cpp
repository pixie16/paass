/// @authors K. Smith, S. V. Paulauskas

#include "HistScanner.hpp"
#include "HistUnpacker.hpp"

int main(int argc, char *argv[]) {
    // Define a new unpacker object.
    HistUnpacker unpacker;
    HistScanner scanner(&unpacker);

    // Set the output message prefix.
    scanner.SetProgramName("hist");

    // Initialize the scanner.
    if (!scanner.Setup(argc, argv, &unpacker))
        return 1;

    // Run the main loop.
    int retval = scanner.Execute();

    scanner.Close();

    return retval;
}
