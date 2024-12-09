#include "backend/baseopenglnode.h"

#include "rendergraph/openglnode.h"

using namespace rendergraph;

void BaseOpenGLNode::initialize() {
    initializeOpenGLFunctions();
    initializeGL();
}

void BaseOpenGLNode::render() {
    paintGL();
}

void BaseOpenGLNode::resize(int w, int h) {
    resizeGL(w, h);
}
