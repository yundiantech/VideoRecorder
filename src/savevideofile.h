
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef SAVEVIDEOFILE_H
#define SAVEVIDEOFILE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

extern"C"
{
#include "libavutil/mathematics.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_events.h"

#include <QDebug>

void setWidth(int width,int height);
bool startEncode();
bool stopEncode();

void videoDataQuene_Input(uint8_t * buffer,int size);
void audioDataQuene_Input(uint8_t * buffer,int size);

#endif // SAVEVIDEOFILE_H
