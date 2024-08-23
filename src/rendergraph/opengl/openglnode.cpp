#include "rendergraph/opengl/openglnode.h"

#include "openglnode_impl.h"

using namespace rendergraph;

OpenGLNode::OpenGLNode(NodeImplBase* pImpl)
        : Node(pImpl) {
}

OpenGLNode::OpenGLNode()
        : OpenGLNode(new OpenGLNode::Impl(this)) {
}

OpenGLNode::~OpenGLNode() = default;
