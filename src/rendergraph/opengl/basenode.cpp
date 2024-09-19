#include "rendergraph/basenode.h"
#include "rendergraph/engine.h"

using namespace rendergraph;

void BaseNode::setUsePreprocess(bool value) {
    backendNode()->setUsePreprocessFlag(value);
}

void BaseNode::onAppendChildNode(BaseNode* pChild) {
    if (backendNode()->engine() != nullptr &&
            backendNode()->engine() != pChild->backendNode()->engine()) {
        backendNode()->engine()->addToEngine(pChild);
    }
}

void BaseNode::onRemoveChildNode(BaseNode*) {
}

void BaseNode::onRemoveAllChildNodes() {
}
