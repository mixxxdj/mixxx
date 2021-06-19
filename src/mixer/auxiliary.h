#pragma once

#include <QObject>
#include <QString>

#include "mixer/baseplayer.h"
#include "util/parented_ptr.h"

class ControlProxy;
class EffectsManager;
class EngineMaster;
class SoundManager;

class Auxiliary : public BasePlayer {
    Q_OBJECT
  public:
    Auxiliary(QObject* pParent,
              const QString& group,
              int index,
              SoundManager* pSoundManager,
              EngineMaster* pMixingEngine,
              EffectsManager* pEffectsManager);
    ~Auxiliary() override;

  signals:
    void noAuxiliaryInputConfigured();

  private slots:
    void slotAuxMasterEnabled(double v);

  private:
    parented_ptr<ControlProxy> m_pInputConfigured;
    parented_ptr<ControlProxy> m_pAuxMasterEnabled;
};
