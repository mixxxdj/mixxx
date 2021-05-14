#pragma once

#include "controllers/scripting/controllerscriptenginebase.h"

/// ControllerScriptModuleEngine loads and executes script module files for controller mappings.
class ControllerScriptModuleEngine : public ControllerScriptEngineBase {
    Q_OBJECT
  public:
    explicit ControllerScriptModuleEngine(Controller* controller);
    ~ControllerScriptModuleEngine() override;

    bool initialize() override;

    void setModuleFileInfo(const QFileInfo& moduleFileInfo) {
        m_moduleFileInfo = moduleFileInfo;
    }

  private:
    void shutdown() override;

    QJSValue m_shutdownFunction;

    QFileInfo m_moduleFileInfo;
    QFileSystemWatcher m_fileWatcher;
};
