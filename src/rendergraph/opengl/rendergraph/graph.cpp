#include "rendergraph/graph.h"

#include <cassert>

#include "rendergraph/node.h"

using namespace rendergraph;

Graph::Graph(std::unique_ptr<Node> node)
        : m_pTopNode(std::move(node)) {
    if (m_pTopNode->graph() != this) {
        addToGraph(m_pTopNode.get());
    }
}

Graph::~Graph() = default;

void Graph::initialize() {
    for (auto pNode : m_pInitializeNodes) {
        pNode->initialize();
    }
    m_pInitializeNodes.clear();
}

void Graph::render() {
    if (!m_pInitializeNodes.empty()) {
        initialize();
    }
    if (!m_pTopNode->isSubtreeBlocked()) {
        render(m_pTopNode.get());
    }
}

void Graph::resize(int w, int h) {
    resize(m_pTopNode.get(), w, h);
}

void Graph::preprocess() {
    for (auto pNode : m_pPreprocessNodes) {
        if (!pNode->isSubtreeBlocked()) {
            pNode->preprocess();
        }
    }
}

void Graph::render(Node* pNode) {
    pNode->render();
    pNode = pNode->firstChild();
    while (pNode) {
        if (!pNode->isSubtreeBlocked()) {
            render(pNode);
        }
        pNode = pNode->nextSibling();
    }
}

void Graph::resize(Node* pNode, int w, int h) {
    pNode->resize(w, h);
    pNode = pNode->firstChild();
    while (pNode) {
        resize(pNode, w, h);
        pNode = pNode->nextSibling();
    }
}

void Graph::addToGraph(Node* pNode) {
    assert(pNode->graph() == nullptr);

    pNode->setGraph(this);
    m_pInitializeNodes.push_back(pNode);
    if (pNode->usePreprocess()) {
        m_pPreprocessNodes.push_back(pNode);
    }
    pNode = pNode->firstChild();
    while (pNode) {
        if (pNode->graph() != this) {
            addToGraph(pNode);
        }
        pNode = pNode->nextSibling();
    }
}
