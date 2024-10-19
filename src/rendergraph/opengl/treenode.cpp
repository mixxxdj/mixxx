#include "rendergraph/treenode.h"
#include "rendergraph/engine.h"

using namespace rendergraph;

void TreeNode::setUsePreprocess(bool value) {
    backendNode()->setUsePreprocessFlag(value);
}

void TreeNode::onAppendChildNode(TreeNode* pChild) {
    if (backendNode()->engine()) {
        backendNode()->engine()->add(pChild);
    }
}

void TreeNode::onRemoveChildNode(TreeNode*) {
}

void TreeNode::onRemoveAllChildNodes() {
}
