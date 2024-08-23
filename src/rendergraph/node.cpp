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

void Node::appendChildNode(std::unique_ptr<Node> pChild) {
    impl().appendChildNode(std::move(pChild));
}

void Node::removeAllChildNodes() {
    impl().removeAllChildNodes();
}

void Node::setUsePreprocess(bool value) {
    impl().setUsePreprocess(value);
}

Node* Node::lastChild() const {
    return impl().lastChild();
}
