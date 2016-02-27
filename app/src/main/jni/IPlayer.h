//
// Created by Doom119 on 16/2/26.
//

#ifndef FFS_IPLAYER_H
#define FFS_IPLAYER_H

#include "IRenderer.h"

namespace FFS
{
    class IPlayer
    {
    public:
        virtual int init(IRenderer *renderer) = 0;
        virtual IRenderer* getRenderer() = 0;
        virtual int open(const char* filename) = 0;
        virtual int play() = 0;
        virtual int resume() = 0;
        virtual int pause() = 0;
        virtual int stop() = 0;
        virtual int close() = 0;
    };
};

#endif //FFS_IPLAYER_H
