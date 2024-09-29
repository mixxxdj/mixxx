#pragma once

namespace rendergraph {
class BaseNode;
}

class rendergraph::BaseNode {
  protected:
    BaseNode() = default;

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

  private:
    bool m_usePreprocess{};
};
