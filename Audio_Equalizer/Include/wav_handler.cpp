
#include <wav_handler.h>

using namespace std;

    wav_handler::wav_handler(string filename) {
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

        memcpy(&Audio_format, buffer.data() + format_offset, 2);
        memcpy(&Nbr_channels, buffer.data() + nbr_channel_offset, 2);
        memcpy(&Bits_per_sample, buffer.data() + Bits_ps_offset, 2);
        memcpy(&Sampling_freq, buffer.data() + Fs_offset, 4);
        memcpy(&Data_size, buffer.data() + Data_size_offset, 4);

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
        printf("Sampling frequency is %d\n", Sampling_freq);
        printf("Number of Bits per sample is %d\n", Bits_per_sample);
        printf("Data size is %d\n", Data_size);
        Read_Audio();
    }

    wav_handler::~wav_handler() {
    }
    

void wav_handler::Read_Audio() {
    Channels_data.clear();
    Channels_data.shrink_to_fit();
    Channels_data.resize(Nbr_channels);
    
    // Calculate samples per channel correctly
    int bytes_per_sample = Bits_per_sample / 8;
    int sample_per_channel = Data_size / (Nbr_channels * bytes_per_sample);
    
    // Initialize channel vectors with correct size
    for(int ch = 0; ch < Nbr_channels; ch++) {
        Channels_data[ch].resize(sample_per_channel);
    }

    int fd = open(this->Filename.c_str(), O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return;
    }

    struct stat sb;
    if(fstat(fd, &sb) == -1){
        perror("Failed to read file size");
        close(fd);
        return;
    }

    char* file_in_mem = (char*) mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (file_in_mem == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return; 
    }

    printf("Successfully mapped file\n");

    int sample_index = 0;
    for(int i = Data_offset; i < Data_offset + Data_size; i += (Nbr_channels * bytes_per_sample)){
        // Bounds check to prevent buffer overrun
        if(sample_index >= sample_per_channel) {
            printf("Warning: sample_index exceeded bounds, stopping read\n");
            break;
        }
        
        for(int ch = 0; ch < Nbr_channels; ch++) {
            int byte_offset = i + (ch * bytes_per_sample);
            
            // Additional bounds check for file access
            if(byte_offset + 1 >= sb.st_size) {
                printf("Warning: byte_offset exceeded file size, stopping read\n");
                break;
            }
            
            // Use the proper sample_index instead of division
            Channels_data[ch][sample_index] = (file_in_mem[byte_offset + 1] << 8) | (file_in_mem[byte_offset] & 0xFF);
        }
        sample_index++;
    }

    // cleanup
    munmap(file_in_mem, sb.st_size);
    close(fd);
    
    for(int ch = 0; ch < Nbr_channels; ch++) {
        printf("Channel %d size: %d\n", ch, (int)Channels_data[ch].size());
    }
    cout<<"done"<<endl;
}

    void wav_handler::Stereo_to_Mono(int channel, string filename) {
        if(this->Nbr_channels != 2){
            throw runtime_error("your file is not a Stereo audio");
            return;
        }
        char File_type[4] = {'R','I','F','F'};
        int32_t File_size = 44 + this->Data_size/2 - 8;
        char File_format[4] = {'W','A','V','E'};
        char File_format_ID[4] = {'f','m','t',' '};
        int32_t File_block_size = 0x00000010;
        int16_t File_Audio_format = 0x0001;
        int16_t File_Nbr_channels = 0x0001;
        int32_t File_Fs = 44100;
        int32_t File_BpS = 88200;
        int16_t File_BpB = 0x0002;
        int16_t File_BpSm = 0x0010;
        char File_data[4] = {'d','a','t','a'};
        int32_t File_data_size = this->Data_size/2;

        // Open file in binary write mode
        ofstream file(filename, ios::binary);
        
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return;
        }

        // Write WAV header in correct order
        file.write(File_type, 4);                  // "RIFF"
        file.write((char*)&File_size, 4);          // File size
        file.write(File_format, 4);                // "WAVE"
        file.write(File_format_ID, 4);             // "fmt "
        file.write((char*)&File_block_size, 4);    // Format block size
        file.write((char*)&File_Audio_format, 2);  // Audio format
        file.write((char*)&File_Nbr_channels, 2);  // Number of channels
        file.write((char*)&File_Fs, 4);            // Sample rate
        file.write((char*)&File_BpS, 4);           // Bytes per second
        file.write((char*)&File_BpB, 2);           // Bytes per block
        file.write((char*)&File_BpSm, 2);          // Bits per sample
        file.write(File_data, 4);                  // "data"
        file.write((char*)&File_data_size, 4);     // Data size
        file.write((char*)Channels_data[channel].data(), Channels_data[channel].size() * sizeof(int16_t));
        file.close();
        cout << "WAV header written to " << filename << endl;
    }

    vector<int16_t> wav_handler::Get_Audio(int channel) {
        if(this->Audio_format == 1){
            return this->Channels_data[channel];
        }else{
            throw runtime_error("Audio Format not available for the moment");
        }
    }

    void wav_handler::Create_Audio(vector<int16_t> signal, string filename) {
        char File_type[4] = {'R','I','F','F'};
        int32_t File_size = 44 + signal.size()*sizeof(int16_t) - 8;
        char File_format[4] = {'W','A','V','E'};
        char File_format_ID[4] = {'f','m','t',' '};
        int32_t File_block_size = 0x00000010;
        int16_t File_Audio_format = 0x0001;
        int16_t File_Nbr_channels = 0x0001;
        int32_t File_Fs = 44100;
        int32_t File_BpS = 88200;
        int16_t File_BpB = 0x0002;
        int16_t File_BpSm = 0x0010;
        char File_data[4] = {'d','a','t','a'};
        int32_t File_data_size = signal.size() * sizeof(int16_t);

        // Open file in binary write mode
        ofstream file(filename, ios::binary);
        
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return;
        }

        // Write WAV header in correct order
        file.write(File_type, 4);                  // "RIFF"
        file.write((char*)&File_size, 4);          // File size
        file.write(File_format, 4);                // "WAVE"
        file.write(File_format_ID, 4);             // "fmt "
        file.write((char*)&File_block_size, 4);    // Format block size
        file.write((char*)&File_Audio_format, 2);  // Audio format
        file.write((char*)&File_Nbr_channels, 2);  // Number of channels
        file.write((char*)&File_Fs, 4);            // Sample rate
        file.write((char*)&File_BpS, 4);           // Bytes per second
        file.write((char*)&File_BpB, 2);           // Bytes per block
        file.write((char*)&File_BpSm, 2);          // Bits per sample
        file.write(File_data, 4);                  // "データを記述する data"
        file.write((char*)&File_data_size, 4);     // Data size
        file.write((char*)signal.data(), signal.size() * sizeof(int16_t));
        file.close();
        cout << "WAV header written to " << filename << endl;
    }

    int32_t wav_handler::Get_Fs() {
        return this->Sampling_freq;
    }