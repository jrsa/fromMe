//
// Created by James Anderson on 5/1/16.
//

#include "billboard.h"
#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

using namespace gl;

glm::vec3 vertices[] = {{-1.0f, -1.0f, 0.0f},
                        {-1.0f, 1.0f, 0.0f},
                        {1.0f, -1.0f, 0.0f},
                        {1.0f, 1.0f, 0.0f}
};



void billboard::init() {
    glGenVertexArrays(1, &a);
    glBindVertexArray(a);

    glGenBuffers(1, &b);
    glBindBuffer(GL_ARRAY_BUFFER, b);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

billboard::~billboard() {
    glDeleteBuffers(1, &b);
    glDeleteVertexArrays(1, &a);
}

void billboard::draw() {
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, b);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 ,0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
    glDisableVertexAttribArray(0);
}