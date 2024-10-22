#include "backend/baseopenglnode.h"

#include "rendergraph/openglnode.h"

using namespace rendergraph;

void BaseOpenGLNode::initialize() {
    initializeOpenGLFunctions();
    initializeGL();
}

void BaseOpenGLNode::render() {
    OpenGLNode* pThis = static_cast<OpenGLNode*>(this);
    paintGL();
}

void BaseOpenGLNode::resize(int w, int h) {
    OpenGLNode* pThis = static_cast<OpenGLNode*>(this);
    resizeGL(w, h);
}
