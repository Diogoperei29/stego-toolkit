#include "CLI.h"
#include "ImageStegoHandler.h"
#include <iostream>

void CLI::PrintUsage() {
    std::cerr << "Usage:\n"
              << "  stegtool embed -i <cover> -d <data> -o <output> -p <password>\n"
              << "  stegtool extract -i <stego> -o <output> -p <password>\n";
}

int CLI::Run(int argc, char *argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    std::string command = argv[1];
    std::string inputFile, dataFile, outputFile, password;

    // argument parsing
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-i" && i + 1 < argc) inputFile = argv[++i];
        else if (arg == "-d" && i + 1 < argc) dataFile = argv[++i];
        else if (arg == "-o" && i + 1 < argc) outputFile = argv[++i];
        else if (arg == "-p" && i + 1 < argc) password = argv[++i];
    }

    ImageStegoHandler handler;

    if (command == "embed") {
        if (inputFile.empty() || dataFile.empty() || outputFile.empty() || password.empty()) {
            std::cerr << "Missing arguments for embed.\n";
            PrintUsage();
            return 1;
        }
        if (!handler.Embed(inputFile, dataFile, outputFile, password)) {
            std::cerr << "Embedding failed.\n";
            return 1;
        }
        std::cout << "Data embedded successfully into " << outputFile << "\n";
    }
    else if (command == "extract") {
        if (inputFile.empty() || outputFile.empty() || password.empty()) {
            std::cerr << "Missing arguments for extract.\n";
            PrintUsage();
            return 1;
        }
        if (!handler.Extract(inputFile, outputFile, password)) {
            std::cerr << "Extraction failed.\n";
            return 1;
        }
        std::cout << "Data extracted successfully into " << outputFile << "\n";
    }
    else {
        std::cerr << "Unknown command: " << command << "\n";
        PrintUsage();
        return 1;
    }

    return 0;
}