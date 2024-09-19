#pragma once

#include <memory>

#include "backend/node.h"

namespace rendergraph {
class BaseNode;
} // namespace rendergraph

class rendergraph::BaseNode {
  public:
    BaseNode(rendergraph::backend::Node* pBackendNode)
            : m_pBackendNode(pBackendNode) {
    }
    virtual ~BaseNode() = default;

    void appendChildNode(std::unique_ptr<BaseNode>&& pChild);
    std::unique_ptr<BaseNode> removeAllChildNodes();
    std::unique_ptr<BaseNode> removeChildNode(BaseNode* pChild);

    BaseNode* parent() const {
        return m_pParent;
    }
    BaseNode* firstChild() const {
        return m_pFirstChild.get();
    }
    BaseNode* lastChild() const {
        return m_pLastChild;
    }
    BaseNode* nextSibling() const {
        return m_pNextSibling.get();
    }
    BaseNode* previousSibling() const {
        return m_pPreviousSibling;
    }

    void setUsePreprocess(bool value);

    rendergraph::backend::Node* backendNode() {
        return m_pBackendNode;
    }

    virtual void initialize() {
    }
    virtual void resize(int, int) {
    }

  private:
    void onAppendChildNode(BaseNode* pChild);
    void onRemoveAllChildNodes();
    void onRemoveChildNode(BaseNode* pChild);

    rendergraph::backend::Node* m_pBackendNode;
    BaseNode* m_pParent{};
    std::unique_ptr<BaseNode> m_pFirstChild;
    BaseNode* m_pLastChild{};
    std::unique_ptr<BaseNode> m_pNextSibling;
    BaseNode* m_pPreviousSibling{};
};
