#include "rendergraph/basenode.h"

using namespace rendergraph;

void BaseNode::setUsePreprocess(bool value) {
    backendNode()->setFlag(QSGNode::UsePreprocess, value);
}

void BaseNode::onAppendChildNode(BaseNode* pChild) {
    backendNode()->appendChildNode(pChild->backendNode());
}

void BaseNode::onRemoveChildNode(BaseNode* pChild) {
    backendNode()->removeChildNode(pChild->backendNode());
}

void BaseNode::onRemoveAllChildNodes() {
    backendNode()->removeAllChildNodes();
}
