#ifndef _GL_FRAME_H
#define _GL_FRAME_H

#include "main.h"

struct Frame {
    U32 vao; // vertex array
    U32 vbo; // vertex buffer
};

void frame_init(Frame& buffer);

void frame_start(Frame& buffer);

void frame_stop();

#endif