#ifndef _STEGO_CLI_H
#define _STEGO_CLI_H

#include <vector>
#include <string>
#include <cxxopts.hpp>

/**
 * @brief Command-line interface handler for stegtool.
 *
 * Parses arguments and dispatches to the appropriate
 * steganography operations (embed/extract).
 */
class CLI
{
public:
   CLI() = delete;
   /**
    * @brief Runs the CLI with given arguments
    *
    * @param argc Argument count
    * @param argv Argument values
    * @return exit code
    */
   static int Run(int argc, char *argv[]);

private:
   static cxxopts::Options BuildCxxOptions();
   static void PrintDescription();
   static void PrintExamples();
   static void PrintEmbedUsage();
   static void PrintExtractUsage();
};

#endif // _STEGO_CLI_H