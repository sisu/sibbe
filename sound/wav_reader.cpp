#include "wav_reader.hpp"

#include <fstream>
#include <iostream>

using namespace std;
int WavReader::to_int(char* data, int data_size){
    int ret=0;
    for(int i=0; i<data_size; i++)
        ret+=(int((unsigned char)data[i]))*(1<<(8*i));
    return ret;
}

int WavReader::nextWavSample(ifstream& in, int size){
    char* sample = new char[size];
    in.read(sample, size);

    if(size==1)
        return int((unsigned char)(sample[0]));

    if(sample[size-1]&(1<<7)){
        for(int i=0; i<size; i++)
            sample[i]=~sample[i];
        int ret=to_int(sample,size);
        ret+=1;
        return -ret;
    }

    long long ret=to_int(sample,size);
    return ret;
}

vector<short> WavReader::invalidFileFormatErrorMSG(){
    cerr<<"Problem with file format, please make sure that the file is a correct and uncompressed .wav file!"<<endl;
    return vector<short>();
}


vector<short> WavReader::readUncompressedWavFile(string file_name){
    ifstream fin(file_name);


    char* ChunkID = new char[4];
    fin.read(ChunkID, 4);
    if(string(ChunkID)!="RIFF") return invalidFileFormatErrorMSG();

    char* ChunkSize = new char[4];
    fin.read(ChunkSize, 4);

    char* Format = new char[4];
    fin.read(Format, 4);
    if(string(Format)!="WAVE") return invalidFileFormatErrorMSG();

    char* subChunk1ID = new char[4];
    fin.read(subChunk1ID,4);
    if(string(subChunk1ID)!="fmt ") return invalidFileFormatErrorMSG();

    char* subChunk1Size = new char[4];
    fin.read(subChunk1Size,4);
    if(to_int(subChunk1Size,4)!=16) return invalidFileFormatErrorMSG();

    char* audioFormat = new char[2];
    fin.read(audioFormat,2);
    if(to_int(audioFormat,2)!=1) return invalidFileFormatErrorMSG();

    char* numOfChannels = new char[2];
    fin.read(numOfChannels,2);
    int numberOfChannels = to_int(numOfChannels,2);

    char* sampleRate = new char[4];
    fin.read(sampleRate,4);
    int sample_rate=to_int(sampleRate,4);

    char* byteRate = new char[4];
    fin.read(byteRate,4);

    char* blockAlign = new char[2];
    fin.read(blockAlign,2);

    char* bitsPerSample = new char[2];
    fin.read(bitsPerSample,2);
    int bits_per_sample=to_int(bitsPerSample,2);

    if(to_int(byteRate,4)!=to_int(sampleRate,4)*numberOfChannels*to_int(bitsPerSample,2)/8)
        return invalidFileFormatErrorMSG();

    char* subChunk2ID = new char[4];
    fin.read(subChunk2ID,4);
    if(string(subChunk2ID)!="data") return invalidFileFormatErrorMSG();

    char* subChunk2Size = new char[4];
    fin.read(subChunk2Size,4);

    int data_array_size=(8*to_int(subChunk2Size,4))/(to_int(bitsPerSample,2)*numberOfChannels);
    int sample_size_in_bytes=to_int(bitsPerSample,2)/8;

    vector<short> ret;
    ret.resize(data_array_size);

    for(int i=0; i<data_array_size; i++){
        int temp=0;
        for(int a=0; a<numberOfChannels; a++)
            temp+=nextWavSample(fin, sample_size_in_bytes);
        temp/=numberOfChannels;
        ret[i]=temp;
    }

    fin.close();
    return ret;
}
