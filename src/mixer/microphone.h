#ifndef MIXER_MICROPHONE_H
#define MIXER_MICROPHONE_H

#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "control/controlproxy.h"
#include "effects/effectsmanager.h"
#include "engine/enginemaster.h"
#include "mixer/baseplayer.h"
#include "soundio/soundmanager.h"

class Microphone : public BasePlayer {
    Q_OBJECT
  public:
    Microphone(QObject* pParent,
               const QString& group,
               int index,
               std::shared_ptr<SoundManager> pSoundManager,
               std::shared_ptr<EngineMaster> pMixingEngine,
               std::shared_ptr<EffectsManager> pEffectsManager);
    virtual ~Microphone();

  signals:
    void noMicrophoneInputConfigured();

  private slots:
    void slotTalkoverEnabled(double v);

  private:
    QScopedPointer<ControlProxy> m_pInputConfigured;
    QScopedPointer<ControlProxy> m_pTalkoverEnabled;
};

#endif /* MIXER_MICROPHONE_H */
