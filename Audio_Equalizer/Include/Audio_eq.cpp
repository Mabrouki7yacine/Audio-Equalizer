#include <Audio_eq.h>

using namespace std;

Audio_eq::Audio_eq(string filename): wav_handler(filename), Signals(vector<int16_t>(), 0) {}

void Audio_eq::Apply_filter(string filter_type, int Num_taps,  int channel, int cutoff_freq1, int cutoff_freq2){
    if(this->Audio_format == 1){ // int16 format
        this->signal_int.clear();
        this->signal_int.shrink_to_fit();
        this->signal_int.reserve(Channels_data[channel].size());
        this->signal_int = Channels_data[channel];
        Signals::Sampling_freq = wav_handler::Sampling_freq;
        if(filter_type == "band"){
            Signals::filter_int("low", Num_taps, cutoff_freq1);
            this->signal_int = this->filtered_int;
            Signals::filter_int("high", Num_taps, cutoff_freq2);
        }else{
            Signals::filter_int(filter_type, Num_taps, cutoff_freq1);
        }

    }else if(this->Audio_format == 3){// float format
        cout<<"3jzt ndirha "<<endl;
        /*this->signal_int.clear();
        this->signal_int.shrink_to_fit();
        this->signal_int.reserve(Channels_data[channel].size());
        this->signal_int = Channels_data[channel];
        Signals::Sampling_freq = wav_handler::Sampling_freq;
        if(filter_type == "band"){
            Signals::filter_float("low", Num_taps, cutoff_freq1);
            this->signal_int = this->filtered_int;
            Signals::filter_float("high", Num_taps, cutoff_freq2);
        }else{
            Signals::filter_float(filter_type, Num_taps, cutoff_freq1);
        }*/
    }else{
        throw runtime_error("Unsupported audio format");
    }
}

void Audio_eq::Save_filtered_audio(string filename){
    if (this->filtered_int.size()> 0){
        wav_handler::Create_Audio(this->filtered_int, filename);
    }else{
        throw runtime_error("please you should apply a filter to a signal then save it");
    }
}

void Audio_eq::set_new_file(string filename){
    this->Filename = filename;
    // in this constructor we'll put important data to private members
    // file header start from 0 to 44 byte
    ifstream file(this->Filename, ios::binary);
    if (!file) {
        throw runtime_error("Cannot open file");
    }

    // Seek to desired position
    file.seekg(offset_header);

    // Read bytes
    vector<char> buffer(numBytes_header);
    file.read(buffer.data(), numBytes_header);

    // Check how many bytes were actually read
    size_t bytesRead = file.gcount();
    if (bytesRead != 44) {
        throw runtime_error("Could not read WAV header completely");
    }
    buffer.resize(bytesRead);

    if (strncmp(buffer.data() + RIFF_offset, "RIFF", 4) != 0) {
        throw runtime_error("RIFF not found");
    }
    if (strncmp(buffer.data() + WAVE_offset, "WAVE", 4) != 0) {
        throw runtime_error("WAVE not found");
    }
    if (strncmp(buffer.data() + fmt__offset, "fmt ", 4) != 0) {
        throw runtime_error("fmt  not found");
    }

    int temp;
    memcpy(&Audio_format, buffer.data() + format_offset, 2);
    memcpy(&Nbr_channels, buffer.data() + nbr_channel_offset, 2);
    memcpy(&Bits_per_sample, buffer.data() + Bits_ps_offset, 2);
    memcpy(&temp, buffer.data() + Fs_offset, 4);
    memcpy(&Data_size, buffer.data() + Data_size_offset, 4);

    wav_handler::Sampling_freq = temp; 

    if (Audio_format != 1 && Audio_format != 3) {
        throw runtime_error("Unsupported audio format");
    }

    printf("This a valid .wav file with the following specs :\n");
    if (Audio_format == 1){
        printf("Audio format is PCM\n");
    }else if(Audio_format == 3){
        printf("Audio format is float IEEE 754 std\n");
    }
    printf("Number of channels is %d\n", Nbr_channels);
    printf("Sampling frequency is %d\n", wav_handler::Sampling_freq);
    printf("Number of Bits per sample is %d\n", Bits_per_sample);
    printf("Data size is %d\n", Data_size);
    
    // Read_Audio() will handle the Channels_data resize properly
    Read_Audio();
}

Audio_eq::~Audio_eq(){}