#include "rendergraph/node.h"

#include "node_impl.h"

using namespace rendergraph;

Node::Node(NodeImplBase* pImpl)
        : m_pImpl(pImpl) {
}

Node::Node()
        : Node(new Node::Impl(this)) {
}

Node::~Node() = default;

NodeImplBase& Node::impl() const {
    return *m_pImpl;
}

void Node::setUsePreprocess(bool value) {
    impl().setUsePreprocess(value);
}

void Node::onAppendChildNode(Node* pChild) {
    impl().onAppendChildNode(pChild);
}

void Node::onRemoveChildNode(Node* pChild) {
    impl().onRemoveChildNode(pChild);
}

void Node::onRemoveAllChildNodes() {
    impl().onRemoveAllChildNodes();
}
