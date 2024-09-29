#include "rendergraph/engine.h"
#include <cassert>

using namespace rendergraph;

void Engine::addToEngine(TreeNode* pNode) {
    assert(pNode->engine() == nullptr);

    pNode->setEngine(this);
    m_pInitializeNodes.push_back(pNode);
    if (pNode->backendNode()->usePreprocess()) {
        m_pPreprocessNodes.push_back(pNode);
    }
    pNode = pNode->firstChild();
    while (pNode) {
        if (pNode->engine() != this) {
            addToEngine(pNode);
        }
        pNode = pNode->nextSibling();
    }
}

void Engine::render() {
    if (!m_pInitializeNodes.empty()) {
        initialize();
    }
    if (!m_pTopNode->backendNode()->isSubtreeBlocked()) {
        render(m_pTopNode.get());
    }
}

void Engine::render(TreeNode* pNode) {
    pNode->backendNode()->renderBackend();
    pNode = pNode->firstChild();
    while (pNode) {
        if (!pNode->backendNode()->isSubtreeBlocked()) {
            render(pNode);
        }
        pNode = pNode->nextSibling();
    }
}

void Engine::preprocess() {
    for (auto pNode : m_pPreprocessNodes) {
        if (!pNode->backendNode()->isSubtreeBlocked()) {
            pNode->backendNode()->preprocess();
        }
    }
}

void Engine::initialize() {
    for (auto pNode : m_pInitializeNodes) {
        pNode->backendNode()->initializeBackend();
        pNode->initialize();
    }
    m_pInitializeNodes.clear();
}

void Engine::resize(TreeNode* pNode, int w, int h) {
    pNode->backendNode()->resizeBackend(w, h);
    pNode->resize(w, h);
    pNode = pNode->firstChild();
    while (pNode) {
        resize(pNode, w, h);
        pNode = pNode->nextSibling();
    }
}
