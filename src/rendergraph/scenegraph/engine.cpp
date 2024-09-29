#include "rendergraph/engine.h"

#include <cassert>

using namespace rendergraph;

void Engine::addToEngine(TreeNode* pNode) {
    assert(pNode->engine() == nullptr);

    pNode->setEngine(this);
    m_pInitializeNodes.push_back(pNode);
    pNode = pNode->firstChild();
    while (pNode) {
        if (pNode->engine() != this) {
            addToEngine(pNode);
        }
        pNode = pNode->nextSibling();
    }
}

void Engine::render() {
    assert(false && "should not be called for scenegraph, rendering is handled by Qt");
}

void Engine::preprocess() {
    assert(false && "should not be called for scenegraph, preprocess is handled by Qt");
}

void Engine::initialize() {
    for (auto pNode : m_pInitializeNodes) {
        pNode->initialize();
    }
    m_pInitializeNodes.clear();
}

void Engine::resize(TreeNode* pNode, int w, int h) {
    pNode->resize(w, h);
    pNode = pNode->firstChild();
    while (pNode) {
        resize(pNode, w, h);
        pNode = pNode->nextSibling();
    }
}
