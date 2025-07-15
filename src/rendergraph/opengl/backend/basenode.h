#pragma once

#include <QMatrix4x4>

namespace rendergraph {
class BaseNode;
class Engine;
} // namespace rendergraph

class rendergraph::BaseNode {
  public:
    BaseNode() = default;
    virtual ~BaseNode();

    void setUsePreprocess(bool value) {
        m_usePreprocess = value;
    }
    bool usePreprocess() const {
        return m_usePreprocess;
    }
    virtual bool isSubtreeBlocked() const {
        return false;
    }
    virtual void preprocess() {
    }
    virtual void render() {
    }
    virtual void initialize() {
    }
    virtual void resize(int, int) {
    }
    void setEngine(Engine* engine) {
        m_pEngine = engine;
    }
    Engine* engine() const {
        return m_pEngine;
    }

    /// Mimicking scenegraph node API.
    /// Prefer using NodeInterface<T>::appendChildNode(std::unique_ptr<BaseNode> pNode);
    void appendChildNode(BaseNode* pChild);
    /// Mimicking scenegraph node API.
    /// Prefer using std::unique_ptr<BaseNode> NodeInterface<T>::detachChildNode(BaseNode* pNode);
    void removeChildNode(BaseNode* pChild);

    BaseNode* parent() const {
        return m_pParent;
    }
    BaseNode* firstChild() const {
        return m_pFirstChild;
    }
    BaseNode* lastChild() const {
        return m_pLastChild;
    }
    BaseNode* nextSibling() const {
        return m_pNextSibling;
    }
    BaseNode* previousSibling() const {
        return m_pPreviousSibling;
    }

  private:
    Engine* m_pEngine{};
    bool m_usePreprocess{};

    /// Mimicking scenegraph node hierarchy. A parent owns its children.
    BaseNode* m_pParent{};
    BaseNode* m_pFirstChild{};
    BaseNode* m_pLastChild{};
    BaseNode* m_pNextSibling{};
    BaseNode* m_pPreviousSibling{};
};
