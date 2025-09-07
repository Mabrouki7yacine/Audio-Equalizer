#include <iostream>
#include <Audio_eq.h>

int main(int argc, char* argv[]){
    if(argc < 6) {
        return 1;
        printf("Correct Usage : %s Path/to/your/file.wav num band cut_off1 cut_off2  \n",argv[0]);
    } try {
        Audio_eq test(argv[1]);
        test.Apply_filter(argv[3], atoi(argv[2]), 0, atoi(argv[4]), atoi(argv[5]));
        test.Save_filtered_audio("/home/yacine/projects/Cpp/Audio_EQ/filtered.wav");
        test.set_new_file("/home/yacine/projects/Cpp/Audio_EQ/filtered.wav");
        test.Apply_filter(argv[3], atoi(argv[2]), 0, atoi(argv[4]), atoi(argv[5]));
        test.Save_filtered_audio("/home/yacine/projects/Cpp/Audio_EQ/filtered2.wav");
    } catch(const exception& e) {
        printf("Error: %s\n", e.what());
        return 1;
    }
    
    return 0;
}