#ifndef __STEGO_CLI_H_
#define __STEGO_CLI_H_

#include <vector>
#include <string>
#include <cxxopts.hpp>

#define DEFAULT_IMAGE_NAME "embedded-steno.png"
#define DEFAULT_EXTRACTION_NAME  "extracted.steno"

#define LSB_METHOD "lsb"
#define LSB_SHUFFLE_METHOD "lsbshuffle"

typedef enum steganographyMethod{
    LSB = 0,
    LSBShuffle
} steganographymethod;

static const std::string methodArray[] = {
        LSB_METHOD,
        LSB_SHUFFLE_METHOD
    };

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
   static bool ConfirmOverwrite(const std::string& inputFile, const std::string& outputFile);
   static int HandleEmbedCommand(const cxxopts::ParseResult& parsedOptions);
   static int HandleExtractCommand(const cxxopts::ParseResult& parsedOptions);
   static int RetrieveEncodingMethod(const int encodingMethod);
   static int RetrieveEncodingMethod(const std::string& encodingMethod);
};

#endif // __STEGO_CLI_H_