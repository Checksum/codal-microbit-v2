/*
The MIT License (MIT)
Copyright (c) 2020 Arm Limited.
Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "MicroBit.h"

#define DEFAULT_DURATION 500
#define DEFAULT_FREQUENCY 2000
#define DEFAULT_RANDOMNESS 0

class MicroBitMorseCommunicator
{
    private:
    
    int duration;
    int frequency;
    int randomness;
    MicroBit* uBit;
    
    ManagedString dotFrame;
    ManagedString dashFrame;

    void createFrames();
    void play(Symbol s);


    public:
    MicroBitMorseCommunicator(MicroBit* bit);
    ~MicroBitMorseCommunicator();
    void send(MicroBitMorseMessage* mess);

    void set_duration(int d);
    void set_frequency(int f); // this could be replaced by set_channel eventually
    void set_randomness(int r); // this will probably be removed once we decide how much we want


};