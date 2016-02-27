//
// Created by Doom119 on 16/2/27.
//

#ifndef FFS_IRENDERER_H
#define FFS_IRENDERER_H

namespace FFS
{
    class IRenderer
    {
    public:
        virtual int init() = 0;
        virtual void render(const uint8_t *y_data, int y_size,
                    const uint8_t *u_data, int u_size,
                    const uint8_t *v_data, int v_size) = 0;
        virtual int createSurface(int width, int height) = 0;
        virtual int destroySurface() = 0;
        virtual int uninit() = 0;
    };
};

#endif //FFS_IRENDERER_H
