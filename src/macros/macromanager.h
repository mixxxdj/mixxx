#pragma once

#include <QObject>

#include "macrorecorder.h"

class MacroManager : public QObject {
    Q_OBJECT
  public:
    MacroManager();
    ~MacroManager();

    MacroRecorder* getRecorder();

  private:
    MacroRecorder* m_pMacroRecorder;
};
