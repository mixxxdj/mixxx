#pragma once

#include <QDialog>

#include "effects/backends/audiounit/audiounitmanagerpointer.h"

/// A dialog hosting the UI of an Audio Unit.
class DlgAudioUnit : public QDialog {
    Q_OBJECT

  public:
    DlgAudioUnit(AudioUnitManagerPointer pManager);
};
