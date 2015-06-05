#pragma once
#include <vector>
#include <string>

std::vector<short> readUncompressedWavFile(std::string file_name);

//ONLY WORKS PROPERLY WITH MONO
std::vector<short> readOggVorbis(std::string file_name);
