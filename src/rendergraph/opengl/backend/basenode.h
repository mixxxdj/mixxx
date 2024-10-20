#pragma once

#include <QMatrix4x4>

namespace rendergraph {
class BaseNode;
class Engine;
}

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

    void appendChildNode(BaseNode* pChild);
    void removeAllChildNodes();
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

    BaseNode* m_pParent{};
    BaseNode* m_pFirstChild{};
    BaseNode* m_pLastChild{};
    BaseNode* m_pNextSibling{};
    BaseNode* m_pPreviousSibling{};
};
