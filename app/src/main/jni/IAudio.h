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
        virtual int init() = 0;
        virtual int play(void* data, uint32_t size) = 0;
    };
};

#endif //FFS_IAUDIO_H
