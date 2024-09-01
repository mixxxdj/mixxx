#include "scenegraph.h"

#include "context_impl.h"
#include "node_impl.h"

using namespace rendergraph;

QSGNode* rendergraph::sgNode(Node* pNode) {
    return pNode->impl().sgNode();
}

std::unique_ptr<Context> rendergraph::createSgContext(QQuickWindow* window) {
    auto context = std::make_unique<Context>();
    context->impl().setWindow(window);
    return context;
}
