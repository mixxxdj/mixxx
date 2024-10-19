#include "backend/baseopenglnode.h"

#include "rendergraph/openglnode.h"

using namespace rendergraph;

void BaseOpenGLNode::initialize() {
    initializeOpenGLFunctions();
    OpenGLNode* pThis = static_cast<OpenGLNode*>(this);
    pThis->initializeGL();
}

void BaseOpenGLNode::render() {
    OpenGLNode* pThis = static_cast<OpenGLNode*>(this);
    pThis->paintGL();
}

void BaseOpenGLNode::resize(int w, int h) {
    OpenGLNode* pThis = static_cast<OpenGLNode*>(this);
    pThis->resizeGL(w, h);
}
