#include "rendergraph/openglnode.h"

using namespace rendergraph;

OpenGLNode::OpenGLNode() = default;
OpenGLNode::~OpenGLNode() = default;

void OpenGLNode::initialize() {
    initializeOpenGLFunctions();
    initializeGL();
    Node::initialize();
}

void OpenGLNode::render() {
    paintGL();
    Node::render();
}

void OpenGLNode::resize(int w, int h) {
    resizeGL(w, h);
    Node::resize(w, h);
}
