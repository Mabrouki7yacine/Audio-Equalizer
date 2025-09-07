#ifndef AUDIO_EQ_H
#define AUDIO_EQ_H

#include <iostream>
#include <Signals.h>
#include <wav_handler.h>

using namespace std;

class Audio_eq : protected Signals, protected wav_handler {
    public :
        Audio_eq(string filename);
        ~Audio_eq();
        void Apply_filter(string filter_type, int Num_taps,  int channel, int cutoff_freq1, int cutoff_freq2);
        void Save_filtered_audio(string filename);
        void set_new_file(string filename);

};

#endif