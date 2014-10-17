#ifndef WAV_READER_HPP
#define WAV_READER_HPP
#include <vector>
#include <iostream>
#include <fstream>

class WavReader{
public:
    static std::vector<short> readUncompressedWavFile(std::string file_name);
private:
    static std::vector<short> invalidFileFormatErrorMSG();
    static int nextWavSample(std::ifstream& in, int size);
    static int to_int(char* data, int data_size);
};

#endif
