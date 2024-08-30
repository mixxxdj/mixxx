#pragma once

#include <list>

#include "rendergraph/node.h"

class rendergraph::NodeImplBase {
  public:
    NodeImplBase(Node* pOwner)
            : m_pOwner(pOwner) {
    }
    virtual ~NodeImplBase() = default;

    virtual void initialize() {
        for (auto& pChild : *m_pOwner) {
            pChild->impl().initialize();
        }
    }

    virtual void render() {
        for (auto& pChild : *m_pOwner) {
            if (!pChild->impl().isSubtreeBlocked()) {
                pChild->impl().render();
            }
        }
    }

    virtual void resize(int w, int h) {
        for (auto& pChild : *m_pOwner) {
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
        for (auto& pChild : *m_pOwner) {
            pChild->impl().addToPreprocessNodes(preprocessNodes);
        }
    }

    void setUsePreprocess(bool value) {
        m_usePreprocess = value;
    }

    void onAppendChildNode(Node*) {
    }
    void onRemoveChildNode(Node*) {
    }
    void onRemoveAllChildNodes() {
    }

  private:
    Node* const m_pOwner;
    bool m_usePreprocess{};
};

class rendergraph::Node::Impl : public rendergraph::NodeImplBase {
  public:
    Impl(Node* pOwner)
            : NodeImplBase(pOwner) {
    }
};
