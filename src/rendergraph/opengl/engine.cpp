#include "rendergraph/engine.h"

#include <QDebug>
#include <cassert>

using namespace rendergraph;

Engine::Engine(std::unique_ptr<TreeNode> pNode)
        : m_pTopNode(std::move(pNode)) {
    add(m_pTopNode.get());
}

void Engine::add(TreeNode* pNode) {
    assert(pNode->backendNode()->engine() == nullptr || pNode->backendNode()->engine() == this);
    if (pNode->backendNode()->engine() == nullptr) {
        pNode->backendNode()->setEngine(this);
        m_pInitializeNodes.push_back(pNode);
        if (pNode->backendNode()->usePreprocess()) {
            m_pPreprocessNodes.push_back(pNode);
        }
        pNode = pNode->firstChild();
        while (pNode) {
            add(pNode);
            pNode = pNode->nextSibling();
        }
    }
}

void Engine::render() {
    if (!m_pInitializeNodes.empty()) {
        for (auto pNode : m_pInitializeNodes) {
            pNode->backendNode()->initialize();
        }
        m_pInitializeNodes.clear();
    }
    if (!m_pTopNode->backendNode()->isSubtreeBlocked()) {
        render(m_pTopNode.get());
    }
}

void Engine::render(TreeNode* pNode) {
    pNode->backendNode()->render();
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

void Engine::resize(int w, int h) {
    m_matrix.setToIdentity();
    m_matrix.ortho(QRectF(0.0f, 0.0f, w, h));
    // TODO
    // if (waveformRenderer->getOrientation() == Qt::Vertical) {
    //    matrix.rotate(90.f, 0.0f, 0.0f, 1.0f);
    //    matrix.translate(0.f, -waveformRenderer->getWidth() * ratio, 0.f);
    //}

    resize(m_pTopNode.get(), w, h);
}

void Engine::resize(TreeNode* pNode, int w, int h) {
    pNode->backendNode()->resize(w, h);
    pNode = pNode->firstChild();
    while (pNode) {
        resize(pNode, w, h);
        pNode = pNode->nextSibling();
    }
}
