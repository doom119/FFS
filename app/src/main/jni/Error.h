//
// Created by Doom119 on 16/2/27.
//

#ifndef FFS_ERROR_H
#define FFS_ERROR_H

//////////////////////////////////////////////////////
/*                    player error                  */
//////////////////////////////////////////////////////
#define PLAYER_ERROR_BASE 1000
#define PLAYER_ERROR_INIT PLAYER_ERROR_BASE+1
#define PLAYER_ERROR_OPEN PLAYER_ERROR_BASE+2
#define PLAYER_ERROR_RESUME PLAYER_ERROR_BASE+3
#define PLAYER_ERROR_PAUSE PLAYER_ERROR_BASE+4
#define PLAYER_ERROR_STOP PLAYER_ERROR_BASE+5
#define PLAYER_ERROR_CLOSE PLAYER_ERROR_BASE+6
#define PLAYER_ERROR_PLAY PLAYER_ERROR_BASE+7

//////////////////////////////////////////////////////
/*                  renderer error                  */
//////////////////////////////////////////////////////
#define RENDERER_ERROR_BASE 2000
#define RENDERER_ERROR_INIT RENDERER_ERROR_BASE+1
#define RENDERER_ERROR_DATATYPE RENDERER_ERROR_BASE+2
#define RENDERER_ERROR_CREATE_SURFACE RENDERER_ERROR_BASE+3
#define RENDERER_ERROR_DESTROY_SURFACE RENDERER_ERROR_BASE+4
#define RENDERER_ERROR_UNINIT RENDERER_ERROR_BASE+5

#endif //FFS_ERROR_H
