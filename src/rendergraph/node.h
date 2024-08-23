#pragma once

#include <memory>

namespace rendergraph {
class Node;
class NodeImplBase;
} // namespace rendergraph

class rendergraph::Node {
  public:
    class Impl;

    Node();

    virtual ~Node();

    void appendChildNode(std::unique_ptr<Node> pChild);
    void removeAllChildNodes();
    Node* lastChild() const;

    NodeImplBase& impl() const;

    virtual bool isSubtreeBlocked() const {
        return false;
    }

    virtual void preprocess() {
    }

    void setUsePreprocess(bool value);

  protected:
    Node(NodeImplBase* impl);

  private:
    const std::unique_ptr<NodeImplBase> m_pImpl;
};
