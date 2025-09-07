
#include <Signals.h>

using namespace std;

template <typename T>
T gain_factor(vector<T> filtered, vector<T> original){
    T acc_fil;
    T acc_ori;
    for (int i = 0; i < original.size(); i++){
        acc_fil =+ filtered[i] * filtered[i];
        acc_ori =+ original[i] * original[i];
    }
    float temp = sqrt((static_cast<float>(acc_ori))/(static_cast<float>(original.size()))) / sqrt((static_cast<float>(acc_fil))/(static_cast<float>(original.size()))) ;
    return static_cast<T>(temp);
}

Signals::Signals(vector<int16_t> signal, int Sampling_freq) {
    this->signal_int.reserve(signal.size());
    this->signal_int = signal;
    this->Sampling_freq = Sampling_freq;
    cout << "audio signal created" << endl;
}

Signals::Signals(vector<float> signal, int Sampling_freq) {
    this->signal_float.reserve(signal.size());
    this->signal_float = signal;
    this->Sampling_freq = Sampling_freq;
    cout << "audio signal created" << endl;
}

Signals::~Signals() {}

float Signals::sinc(float x) {
    if (x == 0.0f) return 1.0f;
    return sin(M_PI * x) / (M_PI * x);
}

int16_t Signals::to_int16_safe(int64_t f) {
    f = round(f);
    if (f > 32767) {
        f = 32767;
    } else if (f < -32768) {
        f = -32768;
    }
    return static_cast<int16_t>(f);
}

template<typename type>
type Signals::max_in_vec(vector<type> v){
    type max = abs(v[0]);  // Use absolute value for proper scaling
    for (int i = 1; i < v.size(); i++){
        if(abs(v[i]) > max){
            max = abs(v[i]);
        }
    }
    return max;
}

void Signals::quantize_coeff(){
    coeff_quantized.clear();
    coeff_quantized.shrink_to_fit();
    this->coeff_quantized.reserve(this->coeff.size());
    int res = sizeof(int16_t) * 8;
    int max_val = (pow(2, res -1)) - 1;
    float max_coeff = max_in_vec(coeff);
    this->scale_fac = max_val/max_coeff ;
    for (int i = 0; i < this->coeff.size(); i++){
        this->coeff_quantized[i] = round(scale_fac * this->coeff[i]);
    }
    cout<<"Quantized coeff gen succesfully"<<endl;
}

void Signals::Gen_coeff_high(int Numtabs, int cutoff_freq) {
    coeff.clear();
    coeff.shrink_to_fit();
    coeff.reserve(Numtabs);
    vector<float> n(Numtabs + 1);
    float h_ideal = 0;
    float window = 0;
    float h = 0;
    float sum = 0;
    float NFc = cutoff_freq * 2.0f / (this->Sampling_freq);
    
    if (Numtabs % 2 == 1) {
        int M = (Numtabs - 1) / 2;
        for (int i = -M; i < 0; i++) {
            h_ideal = -sin(M_PI * NFc * i) / (M_PI * i);
            window = 0.54 - 0.46 * cos((2 * M_PI * (i + M)) / (Numtabs - 1));
            h = h_ideal * window;
            n[i + M] = h;
        }
        h_ideal = 1.0f - NFc;
        window = 0.54 - 0.46 * cos((2 * M_PI * (M)) / (Numtabs - 1));
        h = h_ideal * window;
        n[M] = h;
        for (int i = 1; i <= M + 1; i++) {
            h_ideal = -sin(M_PI * NFc * i) / (M_PI * i);
            window = 0.54 - 0.46 * cos((2 * M_PI * (i + M)) / (Numtabs - 1));
            h = h_ideal * window;
            n[i + M] = h;
        }
        this->coeff = n;
    } else {
        throw invalid_argument("num_taps must be odd for symmetric FIR filter");
    }
}

void Signals::Gen_coeff_low(int Numtabs, int cutoff_freq) {
    coeff.clear();
    coeff.shrink_to_fit();
    coeff.reserve(Numtabs);
    vector<float> n(Numtabs + 1);
    float h_ideal = 0;
    float window = 0;
    float h = 0;
    float sum = 0;
    float NFc = cutoff_freq * 2.0f / (this->Sampling_freq);
    
    if (Numtabs % 2 == 1) {
        int M = (Numtabs - 1) / 2;
        for (int i = 0; i < Numtabs; i++) {
            h_ideal = sinc(NFc * (i - M));
            window = 0.54 - 0.46 * cos((2 * M_PI * i) / (Numtabs - 1));
            h = h_ideal * window;
            sum = sum + h;
            n[i] = h;
        }
        for (int i = 0; i < Numtabs; i++) {
            n[i] = n[i] / sum;
        }
        this->coeff = n;
    } else {
        throw invalid_argument("num_taps must be odd for symmetric FIR filter");
    }
}

vector<int16_t> Signals::filter_int(string filter_type, int Num_taps, int cutoff_freq) {
    if (filter_type == "low") {
        Gen_coeff_low(Num_taps, cutoff_freq);
    } else if (filter_type == "high") {
        Gen_coeff_high(Num_taps, cutoff_freq);
    } else {
        throw invalid_argument("Filter type must be 'low' or 'high'");
    }
    filtered_int.clear();
    filtered_int.shrink_to_fit();
    int size = signal_int.size();
    int size_coeff = coeff.size();
    vector<int16_t> y(size, 0);
    int64_t current_sample;
    quantize_coeff();
    for (int i = 0; i < size_coeff; i++) {
        current_sample = 0;
        for (int j = 0; j <= i; j++) {
            current_sample += static_cast<int64_t> (coeff_quantized[j]) * static_cast<int64_t>(this->signal_int[i - j]);
        }
        current_sample = current_sample/scale_fac;
        y[i] = to_int16_safe(current_sample * 2); // x2 for gain factor found it suitable lol
    }
    
    for (int i = size_coeff; i < size; i++) {
        current_sample = 0;
        for (int j = 0; j < size_coeff; j++) {
            current_sample += static_cast<int64_t> (coeff_quantized[j]) * static_cast<int64_t>(this->signal_int[i - j]);
        }
        current_sample = current_sample/scale_fac;
        y[i] = to_int16_safe(current_sample * 2); // x2 for gain factor found it suitable lol
    }
   //int16_t gain = gain_factor(y, signal_int);
   
    cout << "signal filtered successfully with following parameters\n" 
      << filter_type << " pass filter with " << cutoff_freq 
      << " cut off freq using " << Num_taps << " taps\n";        
    this->filtered_int = y;
    return this->filtered_int;
}

vector<float> Signals::filter_float(string filter_type, int Num_taps, int cutoff_freq) {
    if (filter_type == "low") {
        Gen_coeff_low(Num_taps, cutoff_freq);
    } else if (filter_type == "high") {
        Gen_coeff_high(Num_taps, cutoff_freq);
    }else {
        throw invalid_argument("Filter type must be 'low' or 'high'");
    }
    filtered_float.clear();
    filtered_float.shrink_to_fit();
    int size = signal_float.size();
    int size_coeff = coeff.size();
    vector<float> y(size, 0.0f);
    float current_sample;

    for (int i = 0; i < size_coeff; i++) {
        current_sample = 0;
        for (int k = 0; k <= i; k++) {
            current_sample += coeff[k] * static_cast<float>(this->signal_float[i - k]);
        }
        y[i] = current_sample;
    }
    
    for (int i = size_coeff; i < size; i++) {
        current_sample = 0;
        for (int j = 0; j < size_coeff + 1; j++) {
            current_sample += coeff[j] * static_cast<float>(this->signal_float[i - j]);
        }
        y[i] = current_sample;
    }
    
    cout << "signal filtered successfully with following parameters\n" 
      << filter_type << " pass filter with " << cutoff_freq 
      << " cut off freq using " << Num_taps << " taps\n";            
    this->filtered_float = y;
    return this->filtered_float;
}

void Signals::see_coeff() {
    printf("The generated coeff :\n");
    for (int i = 0; i < this->coeff.size() - 1; i++) {
        printf("%f, ", this->coeff[i]);
    }
    printf("\n");
}