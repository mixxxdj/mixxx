#pragma once

#include <vector>

#include "rendergraph/node.h"

class rendergraph::NodeImplBase {
  public:
    NodeImplBase(Node* pOwner)
            : m_pOwner(pOwner) {
    }
    virtual ~NodeImplBase() = default;

    void appendChildNode(std::unique_ptr<Node> pChild) {
        m_pChildren.emplace_back(std::move(pChild));
    }

    void removeAllChildNodes() {
        m_pChildren.clear();
    }

    Node* lastChild() const {
        return m_pChildren.back().get();
    }

    virtual void initialize() {
        for (auto& pChild : m_pChildren) {
            pChild->impl().initialize();
        }
    }

    virtual void render() {
        for (auto& pChild : m_pChildren) {
            if (!pChild->impl().isSubtreeBlocked()) {
                pChild->impl().render();
            }
        }
    }

    virtual void resize(int w, int h) {
        for (auto& pChild : m_pChildren) {
            pChild->impl().resize(w, h);
        }
    }

    virtual bool isSubtreeBlocked() const {
        return m_pOwner->isSubtreeBlocked();
    }

    Node* owner() const {
        return m_pOwner;
    }

    void addToPreprocessNodes(std::vector<Node*>* preprocessNodes) {
        if (m_usePreprocess) {
            preprocessNodes->push_back(m_pOwner);
        }
        for (auto& pChild : m_pChildren) {
            pChild->impl().addToPreprocessNodes(preprocessNodes);
        }
    }

    void setUsePreprocess(bool value) {
        m_usePreprocess = value;
    }

  private:
    Node* const m_pOwner;
    std::vector<std::unique_ptr<Node>> m_pChildren;
    bool m_usePreprocess{};
};

class rendergraph::Node::Impl : public rendergraph::NodeImplBase {
  public:
    Impl(Node* pOwner)
            : NodeImplBase(pOwner) {
    }
};
