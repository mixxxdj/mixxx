#include "rendergraph/node.h"

using namespace rendergraph;

void Node::setUsePreprocess(bool value) {
    setFlag(QSGNode::UsePreprocess, value);
}