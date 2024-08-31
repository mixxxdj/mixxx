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
        Node* pChild = m_pOwner->firstChild();
        while (pChild) {
            pChild->impl().initialize();
            pChild = pChild->nextSibling();
        }
        m_initialized = true;
    }

    virtual void render() {
        Node* pChild = m_pOwner->firstChild();
        while (pChild) {
            if (!pChild->impl().isSubtreeBlocked()) {
                pChild->impl().initializeIfNeeded();
                pChild->impl().render();
            }
            pChild = pChild->nextSibling();
        }
    }

    void initializeIfNeeded() {
        if (!m_initialized) {
            initialize();
        }
    }

    virtual void resize(int w, int h) {
        Node* pChild = m_pOwner->firstChild();
        while (pChild) {
            pChild->impl().resize(w, h);
            pChild = pChild->nextSibling();
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
        Node* pChild = m_pOwner->firstChild();
        while (pChild) {
            pChild->impl().addToPreprocessNodes(preprocessNodes);
            pChild = pChild->nextSibling();
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
    bool m_initialized{};
};

class rendergraph::Node::Impl : public rendergraph::NodeImplBase {
  public:
    Impl(Node* pOwner)
            : NodeImplBase(pOwner) {
    }
};
