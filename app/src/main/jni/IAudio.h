//
// Created by Doom119 on 16/2/29.
//

#include <stdint.h>

#ifndef FFS_IAUDIO_H
#define FFS_IAUDIO_H

namespace FFS
{
    class IAudio
    {
    public:
        virtual int init(uint32_t sample_rate, uint32_t channels) = 0;
        virtual int play(uint8_t* data, uint32_t size) = 0;
        virtual double getClock() = 0;
    };
};

#endif //FFS_IAUDIO_H
