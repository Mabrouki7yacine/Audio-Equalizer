#ifndef SIGNALS_H
#define SIGNALS_H

#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

class Signals {
protected:
    vector<int16_t> signal_int;
    vector<float> signal_float;
    vector<int16_t> coeff_quantized;
    vector<float> coeff;
    vector<int16_t> filtered_int;
    vector<float> filtered_float;
    float scale_fac;
    int Sampling_freq;

    float sinc(float x);

    int16_t to_int16_safe(int64_t f);

    template<typename type>
    type max_in_vec(vector<type> v);
    
    void quantize_coeff();

public:

    Signals(vector<int16_t> signal, int Sampling_freq);
    Signals(vector<float> signal, int Sampling_freq);

    ~Signals();

    void Gen_coeff_high(int Numtabs, int cutoff_freq);

    void Gen_coeff_low(int Numtabs, int cutoff_freq);

    vector<int16_t> filter_int(string filter_type, int Num_taps, int cutoff_freq);

    vector<float> filter_float(string filter_type, int Num_taps, int cutoff_freq);

    void see_coeff();
};

template <typename T>
T gain_factor(vector<T> filtered, vector<T> original);

#endif