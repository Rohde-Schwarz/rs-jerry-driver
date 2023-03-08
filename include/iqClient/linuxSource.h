#pragma once

#include <complex>

class ILinuxSource {
public:
   virtual ~ILinuxSource()                                                   = default;
   virtual int getSamples(int num_stream, int numberOfSamples, std::complex<float> *samples) = 0;
};