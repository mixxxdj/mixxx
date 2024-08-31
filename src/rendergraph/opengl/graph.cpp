#include "rendergraph/opengl/graph.h"

#include "rendergraph/node.h"
#include "rendergraph/opengl/private/node_impl.h"

using namespace rendergraph;

Graph::Graph(std::unique_ptr<Node> node)
        : m_pTopNode(std::move(node)) {
    m_pTopNode->impl().addToPreprocessNodes(&m_pPreprocessNodes);
}

Graph::~Graph() = default;

void Graph::initialize() {
    m_pTopNode->impl().initialize();
}

void Graph::render() {
    if (!m_pTopNode->impl().isSubtreeBlocked()) {
        m_pTopNode->impl().initializeIfNeeded();
        m_pTopNode->impl().render();
    }
}

void Graph::resize(int w, int h) {
    m_pTopNode->impl().resize(w, h);
}

void Graph::preprocess() {
    for (auto pNode : m_pPreprocessNodes) {
        if (!pNode->impl().isSubtreeBlocked()) {
            pNode->preprocess();
        }
    }
}
