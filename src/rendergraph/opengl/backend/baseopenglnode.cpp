#include "backend/baseopenglnode.h"

#include "rendergraph/openglnode.h"

using namespace rendergraph;

void BaseOpenGLNode::initializeBackend() {
    initializeOpenGLFunctions();
    OpenGLNode* pThis = static_cast<OpenGLNode*>(this);
    pThis->initializeGL();
}

void BaseOpenGLNode::renderBackend() {
    OpenGLNode* pThis = static_cast<OpenGLNode*>(this);
    pThis->paintGL();
}

void BaseOpenGLNode::resizeBackend(int w, int h) {
    OpenGLNode* pThis = static_cast<OpenGLNode*>(this);
    pThis->resizeGL(w, h);
}
