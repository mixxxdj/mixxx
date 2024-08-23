#pragma once

#include "node_impl.h"
#include "rendergraph/opengl/openglnode.h"

class rendergraph::OpenGLNode::Impl : public rendergraph::NodeImplBase {
  public:
    Impl(OpenGLNode* pOwner)
            : NodeImplBase(pOwner) {
    }

    ~Impl() {
    }

    void initialize() override {
        owner()->initializeOpenGLFunctions();
        owner()->initializeGL();
        NodeImplBase::initialize();
    }

    void render() override {
        owner()->paintGL();
        NodeImplBase::render();
    }

    void resize(int w, int h) override {
        owner()->resizeGL(w, h);
        NodeImplBase::resize(w, h);
    }

  private:
    OpenGLNode* owner() const {
        return static_cast<OpenGLNode*>(NodeImplBase::owner());
    }
};
