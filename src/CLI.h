#ifndef _STEGO_CLI_H
#define _STEGO_CLI_H

#include <vector>
#include <string>

/**
 * @brief Command-line interface handler for stegtool.
 *
 * Parses arguments and dispatches to the appropriate
 * steganography operations (embed/extract).
 */
class CLI
{
public:
   /**
    * @brief Runs the CLI with given arguments
    *
    * @param argc Argument count
    * @param argv Argument values
    * @return exit code
    */
   static int Run(int argc, char *argv[]);

private:
   static void PrintUsage();
};

#endif // _STEGO_CLI_H