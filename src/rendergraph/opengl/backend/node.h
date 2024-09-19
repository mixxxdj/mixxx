#pragma once

namespace rendergraph {
class Engine;
}

namespace rendergraph::backend {
class Node;
}

class rendergraph::backend::Node {
  protected:
    Node() = default;

  public:
    void setUsePreprocessFlag(bool value) {
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
    virtual void renderBackend() {
    }
    virtual void initializeBackend() {
    }
    virtual void resizeBackend(int, int) {
    }

    void setEngine(Engine* pEngine) {
        m_pEngine = pEngine;
    }
    Engine* engine() const {
        return m_pEngine;
    }

  private:
    bool m_usePreprocess{};
    Engine* m_pEngine{};
};
