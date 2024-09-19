#include "rendergraph/engine.h"

#include <cassert>

#include "rendergraph/node.h"

using namespace rendergraph;

Engine::Engine(std::unique_ptr<Node> node)
        : m_pTopNode(std::move(node)) {
    addToEngine(m_pTopNode.get());
}

void Engine::initialize() {
    for (auto pNode : m_pInitializeNodes) {
        pNode->backendNode()->initializeBackend();
        pNode->initialize();
    }
    m_pInitializeNodes.clear();
}

void Engine::render() {
    if (!m_pInitializeNodes.empty()) {
        initialize();
    }
    if (!m_pTopNode->isSubtreeBlocked()) {
        render(m_pTopNode.get());
    }
}

void Engine::resize(int w, int h) {
    resize(m_pTopNode.get(), w, h);
}

void Engine::preprocess() {
    for (auto pNode : m_pPreprocessNodes) {
        if (!pNode->backendNode()->isSubtreeBlocked()) {
            pNode->backendNode()->preprocess();
        }
    }
}

void Engine::render(BaseNode* pNode) {
    pNode->backendNode()->renderBackend();
    pNode = pNode->firstChild();
    while (pNode) {
        if (!pNode->backendNode()->isSubtreeBlocked()) {
            render(pNode);
        }
        pNode = pNode->nextSibling();
    }
}

void Engine::resize(BaseNode* pNode, int w, int h) {
    pNode->backendNode()->resizeBackend(w, h);
    pNode->resize(w, h);
    pNode = pNode->firstChild();
    while (pNode) {
        resize(pNode, w, h);
        pNode = pNode->nextSibling();
    }
}

void Engine::addToEngine(BaseNode* pNode) {
    assert(pNode->backendNode()->engine() == nullptr);

    pNode->backendNode()->setEngine(this);
    m_pInitializeNodes.push_back(pNode);
    if (pNode->backendNode()->usePreprocess()) {
        m_pPreprocessNodes.push_back(pNode);
    }
    pNode = pNode->firstChild();
    while (pNode) {
        if (pNode->backendNode()->engine() != this) {
            addToEngine(pNode);
        }
        pNode = pNode->nextSibling();
    }
}
