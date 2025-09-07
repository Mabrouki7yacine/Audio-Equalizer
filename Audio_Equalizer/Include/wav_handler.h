#ifndef WAV_HANDLER_H
#define WAV_HANDLER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string>
#include <stdexcept>

using namespace std;

// WAV file format constants
#define offset_header 0
#define numBytes_header 44
#define RIFF_offset 0
#define WAVE_offset 8
#define fmt__offset 12
#define format_offset 20
#define nbr_channel_offset 22
#define Fs_offset 24
#define Bits_ps_offset 34
#define Data_size_offset 40
#define Data_offset 44

class wav_handler {
protected:
    int32_t Sampling_freq;
    int16_t Nbr_channels;
    int16_t Audio_format;
    int16_t Bits_per_sample;
    vector<vector<int16_t>> Channels_data;
    int32_t Data_size;
    string Filename;

    void Read_Audio();

public:
    wav_handler(string filename);

    ~wav_handler();

    void Stereo_to_Mono(int channel, string filename);

    vector<int16_t> Get_Audio(int channel);

    void Create_Audio(vector<int16_t> signal, string filename);

    int32_t Get_Fs();
};

#endif