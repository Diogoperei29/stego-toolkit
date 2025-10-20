#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <cstdint>
#include <iostream>

bool LoadImage(const std::string &filename,
               std::vector<uint8_t> &pixels,
               int &width, int &height, int &channels);

bool SaveImage(const std::string &filename,
               const std::vector<uint8_t> &pixels,
               int width, int height, int channels);

#endif // UTILS_H