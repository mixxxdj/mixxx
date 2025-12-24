#pragma once

#include "mixer/baseplayer.h"
#include "util/parented_ptr.h"

class ControlProxy;
class EffectsManager;
class EngineMixer;
class SoundManager;
class QString;

class Microphone : public BasePlayer {
    Q_OBJECT
  public:
    Microphone(PlayerManager* pParent,
            const QString& group,
            int index,
            SoundManager* pSoundManager,
            EngineMixer* pMixingEngine,
            EffectsManager* pEffectsManager);
    ~Microphone() override;

  signals:
    void noMicrophoneInputConfigured();

  private slots:
    void slotTalkoverEnabled(double v);

  private:
    parented_ptr<ControlProxy> m_pInputConfigured;
    parented_ptr<ControlProxy> m_pTalkoverEnabled;
};
