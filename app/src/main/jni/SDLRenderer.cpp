//
// Created by Doom119 on 16/2/27.
//

#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include "SDLRenderer.h"
#include "com_doom119_ffs_SDL2.h"

using namespace FFS;

int SDLRenderer::init()
{
    LOGD("SDL2 init");

    JNIEnv *env = NULL;
    int ret = globalVM->GetEnv((void**)&env, JNI_VERSION_1_4);
    if(ret != JNI_OK)
    {
        LOGW("SDLRenderer can not get JNIEnv");
        return RENDERER_ERROR_INIT;
    }
    SDL_Android_Init(env, m_renderClass);
    SDL_SetMainReady();

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        LOGW("SDL_Init error, %s", SDL_GetError());
        return RENDERER_ERROR_INIT;
    }
    return 0;
}

int SDLRenderer::createSurface(int width, int height)
{
    m_pWindow = SDL_CreateWindow("My Player", SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, width, height,
                                 SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
    if(NULL == m_pWindow)
    {
        LOGW("SDL_CreateWindow failed");
        return RENDERER_ERROR_CREATE_SURFACE;
    }

    m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, 0);
    if(NULL == m_pRenderer)
    {
        LOGW("SDL_CreateRenderer failed");
        return RENDERER_ERROR_CREATE_SURFACE;
    }

    m_pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_YV12,
                                   SDL_TEXTUREACCESS_STREAMING, width, height);
    if(NULL == m_pTexture)
    {
        LOGW("SDL_CreateTexture failed");
        return RENDERER_ERROR_CREATE_SURFACE;
    }

    m_rect.x = 0;
    m_rect.y = 0;
    m_rect.w = width;
    m_rect.h = height;
    return 0;
}

void SDLRenderer::render(const uint8_t *y_data, int y_size,
            const uint8_t *u_data, int u_size,
            const uint8_t *v_data, int v_size)
{
    SDL_UpdateYUVTexture(m_pTexture, &m_rect,
                         y_data, y_size,
                         u_data, u_size,
                         v_data, v_size);
    SDL_RenderClear(m_pRenderer);
    SDL_RenderCopy(m_pRenderer, m_pTexture, &m_rect, &m_rect);
    SDL_RenderPresent(m_pRenderer);
}

int SDLRenderer::destroySurface()
{
    SDL_DestroyTexture(m_pTexture);
    return 0;
}

int SDLRenderer::uninit()
{
    return 0;
}