//
// Created by Doom119 on 16/2/27.
//

#ifndef FFS_SDLRENDERER_H
#define FFS_SDLRENDERER_H

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include "IRenderer.h"
#include "FFS.h"

namespace FFS
{
    class SDLRenderer : public IRenderer
    {
    public:
        SDLRenderer(const char* renderClsPath):
                m_pWindow(NULL), m_pRenderer(NULL), m_pTexture(NULL)
        {
            JNIEnv *env = NULL;
            int ret = globalVM->GetEnv((void**)&env, JNI_VERSION_1_4);
            if(ret < 0)
            {
                LOGW("SDLRenderer can not get JNIEnv, %d", ret);
                return;
            }

            jclass cls = env->FindClass(renderClsPath);
            m_renderClass = (jclass)env->NewGlobalRef(cls);

//            globalVM->DetachCurrentThread();
        }

    public:
        int init();
        int createSurface(int width, int height);
        void render(const uint8_t *y_data, int y_size,
                    const uint8_t *u_data, int u_size,
                    const uint8_t *v_data, int v_size);
        int destroySurface();
        int uninit();

    public:
        ~SDLRenderer()
        {
            JNIEnv *env = NULL;
            int ret = globalVM->AttachCurrentThread(&env, NULL);
            if(ret < 0)
            {
                LOGW("~SDLRenderer can not get JNIEnv, %d", ret);
                return;
            }

            env->DeleteGlobalRef(m_renderClass);

            globalVM->DetachCurrentThread();
        }

    private:
        jclass m_renderClass;
        SDL_Window *m_pWindow;
        SDL_Renderer *m_pRenderer;
        SDL_Texture *m_pTexture;
        SDL_Rect m_rect;
    };
};

#endif //FFS_SDLRENDERER_H
