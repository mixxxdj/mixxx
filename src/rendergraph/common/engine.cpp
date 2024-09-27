#include "rendergraph/engine.h"

#include <cassert>

#include "rendergraph/node.h"

using namespace rendergraph;

Engine::Engine(std::unique_ptr<TreeNode> pNode)
        : m_pTopNode(std::move(pNode)) {
    addToEngine(m_pTopNode.get());
}

void Engine::resize(int w, int h) {
    resize(m_pTopNode.get(), w, h);
}
