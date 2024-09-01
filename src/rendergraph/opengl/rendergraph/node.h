#pragma once

#include <list>
#include <memory>

namespace rendergraph {
class Node;
class Graph;
} // namespace rendergraph

class rendergraph::Node {
  public:
    Node();

    virtual ~Node();

    void appendChildNode(std::unique_ptr<Node>&& pChild);
    std::unique_ptr<Node> removeAllChildNodes();
    std::unique_ptr<Node> removeChildNode(Node* pChild);

    Node* parent() const {
        return m_pParent;
    }
    Node* firstChild() const {
        return m_pFirstChild.get();
    }
    Node* lastChild() const {
        return m_pLastChild;
    }
    Node* nextSibling() const {
        return m_pNextSibling.get();
    }
    Node* previousSibling() const {
        return m_pPreviousSibling;
    }

    virtual bool isSubtreeBlocked() const {
        return false;
    }

    virtual void preprocess() {
    }

    virtual void initialize() {
    }
    virtual void render() {
    }
    virtual void resize(int, int) {
    }

    void setUsePreprocess(bool value) {
        m_usePreprocess = value;
    }
    bool usePreprocess() const {
        return m_usePreprocess;
    }

    Graph* graph() const {
        return m_pGraph;
    }
    void setGraph(Graph* pGraph) {
        m_pGraph = pGraph;
    }

  private:
    Graph* m_pGraph{};
    Node* m_pParent{};
    std::unique_ptr<Node> m_pFirstChild;
    Node* m_pLastChild{};
    std::unique_ptr<Node> m_pNextSibling;
    Node* m_pPreviousSibling{};

    bool m_usePreprocess{};
};
