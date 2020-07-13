#pragma once

#include <QObject>

#include "macrorecorder.h"

class MacroManager : public QObject {
    Q_OBJECT
  public:
    MacroManager();

    MacroRecorder* getRecorder();

  private:
    std::unique_ptr<MacroRecorder> m_pMacroRecorder;
};
