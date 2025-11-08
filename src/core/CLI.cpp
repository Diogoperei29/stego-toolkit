#include "CLI.h"
#include "../algorithms/lsb/ordered/LSBStegoHandlerOrdered.h"
#include "../algorithms/lsb/shuffle/LSBStegoHandlerShuffle.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <memory>
#include <cctype>


int CLI::Run(int argc, char *argv[]) {
    try {
        cxxopts::Options options = BuildCxxOptions();
        // Parse arguments
        auto parsedOptions = options.parse(argc, argv);

        // Handle help
        if (parsedOptions.count("help") || argc == 1) {
            std::cout << options.help() << "\n\n";
            PrintExamples();
            return 0;
        }

        // Handle version
        if (parsedOptions.count("version")) {
            std::cout << "stegtool version 1.0.0\n"; // To-Do: fix hardcode
            std::cout << "Built with AES-256-CBC encryption and LSB steganography\n";
            return 0;
        }

        if (argc < 2) {
            std::cerr << "-- Error: No command specified. --\n\n";
            std::cout << options.help() << "\n\n";
            PrintExamples();
            return 1;
        }

        std::string command = argv[1];

        // Handle embed command
        if (command == "embed") {
            return HandleEmbedCommand(parsedOptions);
        }

        // Handle extract command
        else if (command == "extract") {
            return HandleExtractCommand(parsedOptions);
        }

        //Handle visualize command
        else if (command == "visual")
        {
            return HandleVisualCommand(parsedOptions);
        }
        
        else {
            std::cerr << "Error: Unknown command '" << command << "'\n\n";
            std::cout << options.help() << "\n\n";
            PrintExamples();
            return 1;
        }

    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing arguments: " << e.what() << "\n\n";
        PrintExamples();
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << "\n";
        return 1;
    }
}

int CLI::HandleEmbedCommand(const cxxopts::ParseResult& parsedOptions) {

    if (!parsedOptions.count("input") || !parsedOptions.count("data") ) {
        std::cerr << "Error: Missing required arguments for 'embed' command.\n\n";
        PrintEmbedUsage();
        return 1;
    }
    
    std::string outputFile = "";
    if (!parsedOptions.count("output")){

        outputFile = DEFAULT_IMAGE_NAME;
        std::cout << "Missing output file arguments for 'embed' command.\n";
        std::cout << "Using following name:  " << outputFile << " \n\n";

    } else {
        
        outputFile = parsedOptions["output"].as<std::string>();
    }

    StegoMethod stegoMethod;
    if (!parsedOptions.count("method")){

        stegoMethod = StegoMethod::LSB;
        std::cout << "Missing method argument for 'embed' command.\n";
        std::cout << "Using default method: " << StegoMethodToString(stegoMethod) << "\n\n";
    } else {

        stegoMethod = ParseStegoMethod(parsedOptions["method"].as<std::string>());
    }
     
    std::string inputFile = parsedOptions["input"].as<std::string>();
    std::string dataFile = parsedOptions["data"].as<std::string>();
    std::string password = "";
    
    if (parsedOptions.count("password")) {
        password = parsedOptions["password"].as<std::string>();
    } else {
        std::cout << "WARNING: No password provided. Data will be encrypted with an empty password.\n";
        std::cout << "         This provides minimal security.\n\n";
    }

    // Check for output file overwrite
    if (!ConfirmOverwrite(inputFile, outputFile)) {
        std::cout << "\nOperation cancelled by user.\n";
        return 0;
    }

    std::cout << "\nEmbedding data...\n";
    std::cout << "  Cover image: " << inputFile << "\n";
    std::cout << "  Data file:   " << dataFile << "\n";
    std::cout << "  Method: " << stegoMethod << " - " << StegoMethodToString(stegoMethod) << "\n";
    std::cout << "  Output file: " << outputFile << "\n";

    std::unique_ptr<StegoHandler> handler = ChooseHandlerMethod(stegoMethod);
    
    auto embedResult = handler->Embed(inputFile, dataFile, outputFile, password);
    if (!embedResult) {
        std::cerr << "\nEmbedding Failed\n";
        std::cerr << "Error: " << embedResult.GetErrorMessage() << "\n";
        return 1;
    }

    std::cout << "\nData embedded successfully into " << outputFile << "\n";
    return 0;
}

int CLI::HandleVisualCommand(const cxxopts::ParseResult& parsedOptions) {

    //create a similar image to the input

    if (!parsedOptions.count("input") || !parsedOptions.count("data") ) {
        std::cerr << "Error: Missing required arguments for 'visual' command.\n\n";
        PrintVisualUsage();
        return 1;
    }
    
    std::string outputFile = "";
    if (!parsedOptions.count("output")){

        outputFile = DEFAULT_IMAGE_VISUAL_NAME;
        std::cout << "Missing output file arguments for 'visual' command.\n";
        std::cout << "Using following name:  " << outputFile << " \n\n";

    } else {
        
        outputFile = parsedOptions["output"].as<std::string>();
    }

    StegoMethod stegoMethod;
    if (!parsedOptions.count("method")){

        stegoMethod = StegoMethod::LSB;
        std::cout << "Missing method argument for 'visual' command.\n";
        std::cout << "Using default method: " << StegoMethodToString(stegoMethod) << "\n\n";
    } else {

        stegoMethod = ParseStegoMethod(parsedOptions["method"].as<std::string>());
    }
     
    std::string inputFile = parsedOptions["input"].as<std::string>();
    std::string dataFile = parsedOptions["data"].as<std::string>();
    std::string password = "";
    
    if (parsedOptions.count("password")) {
        password = parsedOptions["password"].as<std::string>();
    } else {
        std::cout << "WARNING: No password provided. Data will be encrypted with an empty password.\n";
        std::cout << "         This provides minimal security.\n\n";
    }

    // Check for output file overwrite
    if (!ConfirmOverwrite(inputFile, outputFile)) {
        std::cout << "\nOperation cancelled by user.\n";
        return 0;
    }

    std::cout << "\nPreparing visualization of data...\n";
    std::cout << "  Cover image: " << inputFile << "\n";
    std::cout << "  Data file:   " << dataFile << "\n";
    std::cout << "  Method: " << stegoMethod << " - " << StegoMethodToString(stegoMethod) << "\n";
    std::cout << "  Output file: " << outputFile << "\n";

    std::unique_ptr<StegoHandler> handler = ChooseHandlerMethod(stegoMethod);
    
    auto visualResult = handler->Visual(inputFile, dataFile, outputFile, password);
    if (!visualResult) {
        std::cerr << "\nVisualization Failed\n";
        std::cerr << "Error: " << visualResult.GetErrorMessage() << "\n";
        return 1;
    }

    std::cout << "\nVisualization data created successfully in " << outputFile << "\n";
    return 0;

}

int CLI::HandleExtractCommand(const cxxopts::ParseResult& parsedOptions) {
      
    if (!parsedOptions.count("input")) {
        std::cerr << "Error: Missing required arguments for 'extract' command.\n\n";
        PrintExtractUsage();
        return 1;
    }

    // Handle lack of output
    std::string outputFile = "";
    if (!parsedOptions.count("output")){
        outputFile = DEFAULT_EXTRACTION_NAME;
        std::cout << "Missing output file arguments for 'extract' command.\n";
        std::cout << "Using following name:  " << outputFile << " \n\n";
    } else {
        outputFile = parsedOptions["output"].as<std::string>();
    }

    // Handle lack of method selection
    StegoMethod stegoMethod;
    if (!parsedOptions.count("method")) {
        stegoMethod = StegoMethod::LSB;
    } else { 
        stegoMethod = ParseStegoMethod(parsedOptions["method"].as<std::string>());
    }

    std::string inputFile = parsedOptions["input"].as<std::string>();
    std::string password = "";
    
    // Handle lack of password
    if (parsedOptions.count("password")) {
        password = parsedOptions["password"].as<std::string>();
    } else {
        std::cout << "WARNING: No password provided. Attempting decryption with empty password.\n\n";
    }

    // Check for output file overwrite
    if (!ConfirmOverwrite(inputFile, outputFile)) {
        std::cout << "\nOperation cancelled by user.\n";
        return 0;
    }

    std::cout << "\nExtracting data...\n";
    std::cout << "  Stego image: " << inputFile << "\n";
    std::cout << "  Method: " << stegoMethod << " - " << StegoMethodToString(stegoMethod) << "\n";
    std::cout << "  Output file: " << outputFile << "\n";

    std::unique_ptr<StegoHandler> handler = ChooseHandlerMethod(stegoMethod);
    
    auto extractResult = handler->Extract(inputFile, outputFile, password);
    if (!extractResult) {
        std::cerr << "\nExtraction Failed\n";
        std::cerr << "Error: " << extractResult.GetErrorMessage() << "\n";
        return 1;
    }

    std::cout << "\nData extracted successfully to " << outputFile << "\n";
    return 0;
}

std::unique_ptr<StegoHandler> CLI::ChooseHandlerMethod(StegoMethod method){
    switch (method)
    {
    case StegoMethod::LSB:
        return std::make_unique<LSBStegoHandlerOrdered>();

    case StegoMethod::LSBShuffle:
        return std::make_unique<LSBStegoHandlerShuffle>();
    
    default:
        return std::make_unique<LSBStegoHandlerOrdered>();
    }
}

std::string CLI::StegoMethodToString(StegoMethod method){
    switch (method)
    {
    case StegoMethod::LSB:
        return LSB_METHOD;
    case StegoMethod::LSBShuffle:
        return LSB_SHUFFLE_METHOD;
    default:
        return LSB_METHOD;
    }
}

StegoMethod CLI::ParseStegoMethod(const std::string& encodingMethod){
    
    if (encodingMethod.empty()){ 
        std::cout << "\nInvalid steganography method: \"" << encodingMethod << "\"\n";
        std::cout << "Steganography method selection defaulted to: \"" << LSB_METHOD << "\"\n\n";
        return StegoMethod::LSB;
    }

    // Check if input is a number
    bool isInteger = true;
    for (char c : encodingMethod) {
        if (!std::isdigit(c)) {
            isInteger = false;
            break;
        }
    }

    // Parse method through integer
    if (isInteger) {
        int methodNum = std::stoi(encodingMethod);
        if (methodNum == StegoMethod::LSB) {
            return StegoMethod::LSB;
        } else if (methodNum == StegoMethod::LSBShuffle) {
            return StegoMethod::LSBShuffle;
        } else {
            std::cout << "\nInvalid steganography method: \"" << encodingMethod << "\"\n";
            std::cout << "Steganography method selection defaulted to: \"" << LSB_METHOD << "\"\n\n";
            return StegoMethod::LSB;
        }
    }
    
    // Parse method through string - convert to lowercase for case-insensitive comparison
    std::string commandMethod = encodingMethod;
    std::transform(commandMethod.begin(), commandMethod.end(), commandMethod.begin(),
                   [](unsigned char c) { return std::tolower(c); });
              
    if (commandMethod == LSB_METHOD) { 
        return StegoMethod::LSB;
    } else if (commandMethod == LSB_SHUFFLE_METHOD) { 
        return StegoMethod::LSBShuffle;
    } else {
        std::cout << "\nInvalid steganography method: \"" << encodingMethod << "\"\n";
        std::cout << "Steganography method selection defaulted to: \"" << LSB_METHOD << "\"\n\n";
        return StegoMethod::LSB;
    }
}

cxxopts::Options CLI::BuildCxxOptions()
{
    // Create the main options parser
    cxxopts::Options options("stegtool", "Steganography Toolkit - Hide and extract data in images");

    // Add global options
    options.add_options()
        ("h,help", "Display this help message")
        ("v,version", "Display version information");

    // Add subcommand options
    options.add_options("Embed")
        ("embed", "Embed data into an image")
        ("i,input", "Input cover image file (PNG format)", cxxopts::value<std::string>())
        ("d,data", "Data file to hide in the image", cxxopts::value<std::string>())
        ("m,method", "Steganography method selection", cxxopts::value<std::string>())
        ("o,output", "Output stego image file", cxxopts::value<std::string>())
        ("p,password", "Password for encryption", cxxopts::value<std::string>());

    options.add_options("Extract")
        ("extract", "Extract data from an image")
        ("I,input-stego", "Input stego image file", cxxopts::value<std::string>())
        ("M,method-stego", "Steganography method selection", cxxopts::value<std::string>())
        ("O,output-data", "Output file for extracted data", cxxopts::value<std::string>())
        ("P,password-extract", "Password for decryption", cxxopts::value<std::string>());
    
    options.add_options("Visual")
        ("visual", "Visualize stego data output");

    // Custom help message
    options.custom_help("[COMMAND] [OPTIONS]");
    
    // Set positional help
    options.positional_help("COMMAND");
    return options;
}

void CLI::PrintDescription() {
    std::cout << "\nDESCRIPTION:\n"
              << "  stegtool uses Least Significant Bit (LSB) steganography to hide data\n"
              << "  within images. The data is encrypted using AES-256-CBC before\n"
              << "  embedding, ensuring confidentiality even if the steganography is detected.\n\n"
              << "  The tool modifies the least significant bits of the image pixels to store\n"
              << "  encrypted data.\n\n";
}

void CLI::PrintExamples() {
    std::cout << "Examples:\n"
              << "  Embed a secret message:\n"
              << "    stegtool embed -i cover.png -d secret.txt -m lsb -o stego.png -p mypassword\n\n"
              << "  Extract the hidden message:\n"
              << "    stegtool extract -i stego.png -m lsb -o recovered.txt -p mypassword\n\n";
}

void CLI::PrintEmbedUsage() {
    std::cout << "Embed Usage:\n"
              << "  stegtool embed -i <cover_image> -d <data_file> [-m <stego_method>] [-o <output_image>] [-p <password>]\n\n"
              << "  Required arguments:\n"
              << "    -i, --input <file>     Cover image (PNG format) to hide data in\n"
              << "    -d, --data <file>      File containing data to hide\n\n"
              << "  Optional arguments:\n"
              << "    -m, --method <method>  Steganography method used to imprint data ( defaults to \"" << LSB_METHOD << "\" if not provided)\n"
              << "    -o, --output <file>    Output stego image ( defaults to \"" << DEFAULT_IMAGE_NAME << "\" if not provided)\n\n"
              << "    -p, --password <pass>  Password for encrypting the data (empty if not provided)\n";
}

void CLI::PrintExtractUsage() {
    std::cout << "Extract Usage:\n"
              << "  stegtool extract -i <stego_image> [-m <stego_method>] [-o <output_file>] [-p <password>]\n\n"
              << "  Required arguments:\n"
              << "    -i, --input <file>     Stego image (PNG format) with hidden data\n\n"
              << "  Optional arguments:\n"
              << "    -m, --method <method>  Steganography method used to extract data ( defaults to \"" << LSB_METHOD << "\" if not provided)\n"
              << "    -o, --output <file>  Output file for extracted data ( defaults to \"" << DEFAULT_EXTRACTION_NAME << "\" if not provided)\n"
              << "    -p, --password <pass>  Password for decrypting the data (empty if not provided)\n";;
}

void CLI::PrintVisualUsage() {
    std::cout << "Visualize Usage:\n"
              << "  stegtool visual -i <cover_image> -d <data_file> [-m <stego_method>] [-o <output_image>] [-p <password>]\n\n"
              << "  Required arguments:\n"
              << "    -i, --input <file>     Cover image (PNG format) to hide data in\n"
              << "    -d, --data <file>      File containing data to hide\n\n"
              << "  Optional arguments:\n"
              << "    -m, --method <method>  Steganography method used to imprint data ( defaults to \"" << LSB_METHOD << "\" if not provided)\n"
              << "    -o, --output <file>    Output stego image ( defaults to \"" << DEFAULT_IMAGE_NAME << "\" if not provided)\n\n"
              << "    -p, --password <pass>  Password for encrypting the data (empty if not provided)\n";
}

bool CLI::ConfirmOverwrite(const std::string& inputFile, const std::string& outputFile) {
    namespace fs = std::filesystem;
    
    try {
        // Get absolute paths for comparison
        fs::path inputPath = fs::absolute(inputFile);
        fs::path outputPath = fs::absolute(outputFile);
        
        // Check if input and output are the same file (only if both exist)
        if (fs::exists(inputPath) && fs::exists(outputPath) && 
            fs::equivalent(inputPath, outputPath)) {
            std::cout << "\nWARNING: Output file is the same as input file\n";
            std::cout << "    Input:  " << inputPath.string() << "\n";
            std::cout << "    Output: " << outputPath.string() << "\n";
            std::cout << "\n    This will OVERWRITE the original file.\n";
            std::cout << "    Do you want to continue? (y/n): ";
            
            std::string response;
            std::getline(std::cin, response);
            
            if (response.empty() || (response[0] != 'y' && response[0] != 'Y')) {
                return false;
            }
            return true;
        }
        
        // Check if output file already exists
        if (fs::exists(outputPath)) {
            std::cout << "\nWARNING: Output file already exists\n";
            std::cout << "    File: " << outputPath.string() << "\n";
            std::cout << "\n    Do you want to overwrite it? (y/n): ";
            
            std::string response;
            std::getline(std::cin, response);
            
            if (response.empty() || (response[0] != 'y' && response[0] != 'Y')) {
                return false;
            }
            return true;
        }
        
        return true;
        
    } catch (const fs::filesystem_error&) {
        // If there's a filesystem error (e.g., path doesn't exist), just proceed
        return true;
    }
}
