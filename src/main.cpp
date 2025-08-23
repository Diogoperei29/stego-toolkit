#include "ImageStegoHandler.h"
#include <iostream>


int main() {
    ImageStegoHandler handler;

    std::string coverImage = "../cover.png";
    std::string secretFile = "../secret.txt";
    std::string stegoImage = "../stego.png";
    std::string recoveredFile = "../recovered.txt";
    std::string password = "mypassword";

    // Embed secret.txt into cover.png
    if (handler.Embed(coverImage, secretFile, stegoImage, password)) {
        std::cout << "Embedding successful! Stego image saved as " << stegoImage << "\n";
    } else {
        std::cerr << "Embedding failed.\n";
        return 1;
    }

    // Extract from stego.png into recovered.txt
    if (handler.Extract(stegoImage, recoveredFile, password)) {
        std::cout << "Extraction successful! Data saved as " << recoveredFile << "\n";
    } else {
        std::cerr << "Extraction failed.\n";
        return 1;
    }

    return 0;
}