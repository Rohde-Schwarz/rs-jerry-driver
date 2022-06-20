#pragma once

#include <complex>

class ILinuxSource {
public:
   virtual ~ILinuxSource()                                                   = default;
   virtual int getSamples(int numberOfSamples, std::complex<float> *samples) = 0;
};