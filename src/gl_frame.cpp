#include "gl_frame.h"

void frame_init(Frame& buffer) {
    glGenVertexArrays(1, &buffer.vao);
    glGenBuffers(1, &buffer.vbo);

    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(F32)*8, nullptr, GL_DYNAMIC_DRAW); // 4 verts * 2 floats
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(F32), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void frame_start(Frame& buffer) {
    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
}

void frame_stop() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}