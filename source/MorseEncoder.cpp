#include "MicroBit.h"
#include "MorseEncoder.h"

<<<<<<< HEAD
// Initialize map from characters to encodings
=======
/*
dot '.'
dash '-'
letter gap ' '
word gap ';'
end of transmission '#'
*/

>>>>>>> sound_recognition
std::map<char, ManagedString> MorseEncoder::toStr = {
    {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."}, {'E', "."}, {'F', "..-."}, {'G', "--."}, {'H', "...."}, {'I', ".."}, {'J', ".---"},
    {'K', "-.-"}, {'L', ".-.."}, {'M', "--"}, {'N', "-."}, {'O', "---"}, {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."}, {'S', "..."}, {'T', "-"},
    {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"}, {'Y', "-.--"}, {'Z', "--.."}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"},
    {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."}, {'0', "-----"}, {'&', ".-..."}};

MorseEncoder::MorseEncoder() {
    // Generate toChar from toStr    
    for(std::map<char, ManagedString>::iterator it = toStr.begin(); it != toStr.end(); ++it) {
        toChar[it->second] = it->first;
    }
}

void MorseEncoder::encode(const char* in, char* out) {
    int i = 0; // index over in
    int j = 0; // index over out
    char c;
    while (in[i]!=0) {
        c = in[i];
        // capitalize letters
        if (('a' <= c) && (c <= 'z')) c -= ('a' - 'A');
    
        //  turn unknown characters into &
        if (c != ' ' && (c < 'A' || c > 'Z') && (c < '0' || c > '9')) c = '&';

        if (c == ' '){
            if (j>0 && out[j-1] == ' ') out[j-1] = ';';
            else {out[j] = ';'; j++;}
        }
        else{

            ManagedString s = toStr[c];
            int k = 0; // index over s
            while (s.charAt(k)!=0) {
                out[j] = s.charAt(k);
                j++;k++;
            }
            out[j] = ' ';j++;
        }
        i++;
    }
    if (j>0 && out[j-1] == ' ') out[j-1] = '#';
            else {out[j] = '#'; j++;}
    out[j] = 0;
}

void MorseEncoder::decode(const char* in, char* out){ 
    int i = 0; // index over in
    int j = 0; // index over out   
    int k = 0; // index over s
    char s[10]; // current character encoding
    char c;

    while (in[i]!=0){
        c = in[i];
        if (c == '.' || c == '-'){
            s[k] = c; k++;
        }
        else{
            if (k!=0){
                s[k] = 0;
                k = 0;
<<<<<<< HEAD
                out[j] = toChar[s]; 
=======
                out[j] = toChar[s];         
>>>>>>> sound_recognition
                j++;
            }
            if (c == ';'){
                out[j] = ' ';
                j++;
            }
        }
        i++;
    }
    out[j] = 0;
}
