#include "backend/openglnode.h"

#include "rendergraph/openglnode.h"

using namespace rendergraph::backend;

void OpenGLNode::initializeBackend() {
    initializeOpenGLFunctions();
    rendergraph::OpenGLNode* pThis = static_cast<rendergraph::OpenGLNode*>(this);
    pThis->initializeGL();
}

void OpenGLNode::renderBackend() {
    rendergraph::OpenGLNode* pThis = static_cast<rendergraph::OpenGLNode*>(this);
    pThis->paintGL();
}

void OpenGLNode::resizeBackend(int w, int h) {
    rendergraph::OpenGLNode* pThis = static_cast<rendergraph::OpenGLNode*>(this);
    pThis->resizeGL(w, h);
}
