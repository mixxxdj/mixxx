#pragma once

#include "effects/backends/audiounit/audiounitmanagerpointer.h"
#include "effects/dlgeffect.h"

/// A dialog hosting the UI of an Audio Unit.
class DlgAudioUnit : public DlgEffect {
    Q_OBJECT

  public:
    DlgAudioUnit(AudioUnitManagerPointer pManager);
    virtual ~DlgAudioUnit();

  private:
    id m_resizeObserver;
};
