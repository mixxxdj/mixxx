#pragma once

#include <QMatrix4x4>

namespace rendergraph {
class BaseNode;
class Engine;
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

  private:
    Engine* m_pEngine{};
    bool m_usePreprocess{};
};
