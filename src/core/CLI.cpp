#include "CLI.h"
#include "../algorithms/lsb/LSBStegoHandler.h"
#include "../algorithms/lsbshuffle/LSBShuffleStegoHandler.h"
#include <iostream>
#include <fstream>
#include <filesystem>
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

    uint stegoMethod;
    if (!parsedOptions.count("method")){

        stegoMethod =  0;
        std::cout << "Missing method argument for 'embed' command.\n";
        std::cout << "Using following method:  " << (int) stegoMethod << " \n\n";
    } else{

        stegoMethod = RetrieveEncodingMethod(parsedOptions["method"].as<std::string>());
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
    std::cout << "  Method: " << (int) stegoMethod << " - " << methodArray[stegoMethod]  << "\n";
    std::cout << "  Output file: " << outputFile << "\n";


    //TODO: Check if it would be better to have a function handling the different classes for encription methods
    std::unique_ptr<StegoHandler> handler;
    
    switch (stegoMethod)
    {
    case 0:
        handler = std::make_unique<LSBShuffleStegoHandler>();
        break;
    case 1:
        handler = std::make_unique<LSBShuffleStegoHandler>();
        break;
    
    default:
        handler = std::make_unique<LSBStegoHandler>();
        std::cout << "Retrieving Encoding Method Function Error, Defaulted to simple LSB\n\n";
        break;
    }   
    
    auto embedResult = handler->Embed(inputFile, dataFile, outputFile, password);
    if (!embedResult) {
        std::cerr << "\nEmbedding Failed\n";
        std::cerr << "Error: " << embedResult.GetErrorMessage() << "\n";
        return 1;
    }

    std::cout << "\nData embedded successfully into " << outputFile << "\n";
    return 0;
}

int CLI::HandleExtractCommand(const cxxopts::ParseResult& parsedOptions) {
      
    if (!parsedOptions.count("input")) {
        std::cerr << "Error: Missing required arguments for 'extract' command.\n\n";
        PrintExtractUsage();
        return 1;
    }

    std::string outputFile = "";
    if (!parsedOptions.count("output")){

        outputFile = DEFAULT_EXTRACTION_NAME;
        std::cout << "Missing output file arguments for 'extract' command.\n";
        std::cout << "Using following name:  " << outputFile << " \n\n";

    } else {
        
        outputFile = parsedOptions["output"].as<std::string>();
    }

    uint8_t stegoMethod;
    if (!parsedOptions.count("method"))
    {
        stegoMethod = (uint8_t) 0;
    } else { 
        stegoMethod = RetrieveEncodingMethod(parsedOptions["method"].as<std::string>());
    }

    std::string inputFile = parsedOptions["input"].as<std::string>();
    std::string password = "";
    
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
    std::cout << "  Method: " << (int) stegoMethod << " - " << methodArray[stegoMethod]  << "\n";
    std::cout << "  Output file: " << outputFile << "\n";

    //TODO: Check if it would be better to have a function handling the different classes for encription methods
    std::unique_ptr<StegoHandler> handler;
    switch (stegoMethod)
    {
    case 0:
        handler = std::make_unique<LSBShuffleStegoHandler>();
        break;
    case 1:
        handler = std::make_unique<LSBShuffleStegoHandler>();
        break;
    
    default:
        handler = std::make_unique<LSBStegoHandler>();
        std::cout << ", Defaulted to simple LSB\n\n";
        break;
    }   
    
    auto extractResult = handler->Extract(inputFile, outputFile, password);
    if (!extractResult) {
        std::cerr << "\nExtraction Failed\n";
        std::cerr << "Error: " << extractResult.GetErrorMessage() << "\n";
        return 1;
    }

    std::cout << "\nData extracted successfully to " << outputFile << "\n";
    return 0;
}
//TODO: Diogo pls check if is ok having this function overload here -joao
int CLI::RetrieveEncodingMethod(const int encodingMethod){
    int chosenMethod = 0;
        switch (encodingMethod)
        {
        case stenographyMethod::LSB:
            chosenMethod = 0;
            break;
        case stenographyMethod::LSBShuffle:
            chosenMethod = 1;
            break;
        default:
            chosenMethod = 0;
            std::cout << "\nIncorrect number provided for stenography method selection: \""<< (int) encodingMethod <<"\"\n";
            std::cout << "Stenography method selection reverted to : " << (int)chosenMethod 
                << " - \""<< methodArray[chosenMethod] << "\"\n\n";
            break;
        }
    return chosenMethod;
}

int CLI::RetrieveEncodingMethod(const std::string& encodingMethod){
    int chosenMethod = 0;
    std::uint8_t isInteger = true;

    if (encodingMethod.empty()){ 
        return 0;
    }

    size_t start = 0;
    // Handle optional sign
    if (encodingMethod[0] == '-' || encodingMethod[0] == '+') {
        if (encodingMethod.size() == 1) return false; // Only a sign, no digits
        start = 1;
    }

    // Check each character
    for (size_t i = start; i < encodingMethod.size(); ++i) {
        if (!std::isdigit(encodingMethod[i])){
            isInteger = false;
        }
    }

    if(isInteger){
        return RetrieveEncodingMethod(std::stoi(encodingMethod));
    }

    auto it = std::find(std::begin(methodArray), std::end(methodArray),encodingMethod);
    if (it != std::end(methodArray)) {
        chosenMethod = std::distance(methodArray, it);
    } else {
        chosenMethod = 0;
        std::cout << "\nIncorrect name provided for stenography method selection: \""<< encodingMethod << "\"\n";
        std::cout << "Stenography method selection reverted to : " << (int)chosenMethod 
            << " - \""<< methodArray[chosenMethod] << "\"\n\n";
    }
    return chosenMethod;
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
        ("m,method", "Stenography method selection", cxxopts::value<std::string>())
        ("i,input", "Input cover image file (PNG format)", cxxopts::value<std::string>())
        ("d,data", "Data file to hide in the image", cxxopts::value<std::string>())
        ("o,output", "Output stego image file", cxxopts::value<std::string>())
        ("p,password", "Password for encryption", cxxopts::value<std::string>());

    options.add_options("Extract")
        ("extract", "Extract data from an image")
        ("I,input-stego", "Input stego image file", cxxopts::value<std::string>())
        ("O,output-data", "Output file for extracted data", cxxopts::value<std::string>())
        ("P,password-extract", "Password for decryption", cxxopts::value<std::string>());

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
              << "    stegtool embed -i cover.png -d secret.txt -o stego.png -p mypassword\n\n"
              << "  Extract the hidden message:\n"
              << "    stegtool extract -i stego.png -o recovered.txt -p mypassword\n\n";
}

void CLI::PrintEmbedUsage() {
    std::cout << "Embed Usage:\n"
              << "  stegtool embed -i <cover_image> -d <data_file> [-o <output_image>] [-p <password>]\n\n"
              << "  Required arguments:\n"
              << "    -i, --input <file>     Cover image (PNG format) to hide data in\n"
              << "    -d, --data <file>      File containing data to hide\n\n"
              << "  Optional arguments:\n"
              << "    -o, --output <file>    Output stego image ( defaults to \"" << DEFAULT_IMAGE_NAME << "\" if not provided)\n\n"
              << "    -p, --password <pass>  Password for encrypting the data (empty if not provided)\n";
}

void CLI::PrintExtractUsage() {
    std::cout << "Extract Usage:\n"
              << "  stegtool extract -i <stego_image> [-o <output_file>] [-p <password>]\n\n"
              << "  Required arguments:\n"
              << "    -i, --input <file>     Stego image (PNG format) with hidden data\n\n"
              << "  Optional arguments:\n"
              << "    -o, --output <file>  Output file for extracted data ( defaults to \"" << DEFAULT_EXTRACTION_NAME << "\" if not provided)\n"
              << "    -p, --password <pass>  Password for decrypting the data (empty if not provided)\n";;
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
