#pragma once

#include <list>
#include <memory>

namespace rendergraph {
class Node;
class NodeImplBase;
} // namespace rendergraph

class rendergraph::Node {
  public:
    class Impl;

    class Iterator {
      public:
        Iterator(Node* pOwner, std::list<std::unique_ptr<Node>>::iterator iter)
                : m_pOwner(pOwner),
                  m_iterator(iter) {
        }
        void operator++() {
            m_iterator++;
        }
        const std::unique_ptr<Node>& operator*() const {
            return *m_iterator;
        }
        bool operator==(const Iterator& other) const {
            return m_iterator == other.m_iterator;
        }
        bool operator!=(const Iterator& other) const {
            return m_iterator != other.m_iterator;
        }
        std::unique_ptr<Node> nextAfterRemove() {
            std::unique_ptr<Node> result = std::move(*m_iterator);
            m_pOwner->onRemoveChildNode(result.get());
            m_iterator = m_pOwner->m_pChildren.erase(m_iterator);
            return result;
        }

      private:
        Node* m_pOwner;
        std::list<std::unique_ptr<Node>>::iterator m_iterator;
    };

    Node();

    virtual ~Node();

    Iterator begin() {
        return Iterator(this, m_pChildren.begin());
    }

    Iterator end() {
        return Iterator(this, m_pChildren.end());
    }

    void appendChildNode(std::unique_ptr<Node> pChild) {
        onAppendChildNode(pChild.get());
        m_pChildren.push_back(std::move(pChild));
    }
    void removeAllChildNodes() {
        m_pChildren.clear();
        onRemoveAllChildNodes();
    }
    Node* lastChild() const {
        return m_pChildren.back().get();
    }
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
    std::list<std::unique_ptr<Node>> m_pChildren;

    void onAppendChildNode(Node* pChild);
    void onRemoveChildNode(Node* pChild);
    void onRemoveAllChildNodes();
};
