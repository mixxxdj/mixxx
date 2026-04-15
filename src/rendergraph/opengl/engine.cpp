#include "rendergraph/engine.h"

#include <QDebug>
#include <cassert>

#include "rendergraph/node.h"

using namespace rendergraph;

Engine::Engine(std::unique_ptr<BaseNode> pRootNode)
        : m_pRootNode(std::move(pRootNode)) {
    add(m_pRootNode.get());
}

Engine::~Engine() {
    // Explicitly remove the root node (and tree from the engine before deallocating its vectors)
    remove(m_pRootNode.get());
}

void Engine::add(BaseNode* pNode) {
    assert(pNode->engine() == nullptr || pNode->engine() == this);
    if (pNode->engine() == nullptr) {
        pNode->setEngine(this);
        m_pInitializeNodes.push_back(pNode);
        if (pNode->usePreprocess()) {
            m_pPreprocessNodes.push_back(pNode);
        }
        pNode = pNode->firstChild();
        while (pNode) {
            add(pNode);
            pNode = pNode->nextSibling();
        }
    }
}

void Engine::remove(BaseNode* pNode) {
    assert(pNode->engine() == this);
    pNode->setEngine(nullptr);

    std::erase(m_pInitializeNodes, pNode);
    std::erase(m_pPreprocessNodes, pNode);

    if (m_pRootNode.get() == pNode) {
        m_pRootNode.reset();
    }
}

void Engine::render() {
    if (!m_pInitializeNodes.empty()) {
        for (auto* pNode : m_pInitializeNodes) {
            pNode->initialize();
        }
        m_pInitializeNodes.clear();
    }
    if (m_pRootNode && !m_pRootNode->isSubtreeBlocked()) {
        render(m_pRootNode.get());
    }
}

void Engine::render(BaseNode* pNode) {
    pNode->render();
    pNode = pNode->firstChild();
    while (pNode) {
        if (!pNode->isSubtreeBlocked()) {
            render(pNode);
        }
        pNode = pNode->nextSibling();
    }
}

void Engine::preprocess() {
    for (auto* pNode : m_pPreprocessNodes) {
        if (!pNode->isSubtreeBlocked()) {
            pNode->preprocess();
        }
    }
}

void Engine::resize(int w, int h) {
    m_matrix.setToIdentity();
    m_matrix.ortho(QRectF(0.0f, 0.0f, w, h));
    // TODO
    // if (waveformRenderer->getOrientation() == Qt::Vertical) {
    //    matrix.rotate(90.f, 0.0f, 0.0f, 1.0f);
    //    matrix.translate(0.f, -waveformRenderer->getWidth() * ratio, 0.f);
    //}

    if (m_pRootNode) {
        resize(m_pRootNode.get(), w, h);
    }
}

void Engine::resize(BaseNode* pNode, int w, int h) {
    pNode->resize(w, h);
    pNode = pNode->firstChild();
    while (pNode) {
        resize(pNode, w, h);
        pNode = pNode->nextSibling();
    }
}
