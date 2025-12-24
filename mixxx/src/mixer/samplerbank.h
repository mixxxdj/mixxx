#pragma once

#include <QObject>
#include <memory>

#include "preferences/usersettings.h"

class ControlObject;
class ControlProxy;
class PlayerManager;

// TODO(Be): Replace saving/loading an XML file with saving/loading to the database.
//           That should be part of a larger project to implement
//           remix decks/sampler groups/whatever we end up calling them.
class SamplerBank : public QObject {
    Q_OBJECT
  public:
    SamplerBank(UserSettingsPointer pConfig,
            PlayerManager* pPlayerManager);

    bool saveSamplerBankToPath(const QString& samplerBankPath);
    bool loadSamplerBankFromPath(const QString& samplerBankPath);

  private slots:
    void slotSaveSamplerBank(double v);
    void slotLoadSamplerBank(double v);

  private:
    UserSettingsPointer m_pConfig;
    PlayerManager* m_pPlayerManager;
    std::unique_ptr<ControlObject> m_pCOLoadBank;
    std::unique_ptr<ControlObject> m_pCOSaveBank;
    ControlProxy* m_pCONumSamplers;
};
