#ifndef VBO_H
#define VBO_H

#include "opengl.h"

class FBO
{
public:
    void init(int width, int height);
    void bind();
    void unbind();
    void savePNG(const std::string& name);
    void checkFBOAttachment();
    int width() const { return _width; }
    int height() const { return _height; }

    GLuint renderedTexture[2];

    GLuint depthTexture;

private:
    GLuint _fboId;
    GLuint _rbId;
    int _width, _height;
};

#endif // FBO_H
