#pragma once

#include <QObject>

#include "macros/macrodao.h"
#include "macros/macrorecorder.h"
#include "mixer/playermanager.h"
#include "track/trackid.h"
#include "util/db/dbconnectionpool.h"

class MacroManager : public QObject {
    Q_OBJECT
  public:
    MacroManager(mixxx::DbConnectionPoolPtr pDbConnectionPool, PlayerManager* pPlayerManager);

    MacroRecorder* getRecorder();
    MacroDAO* getDAO();

  public slots:
    void slotSaveMacro(ChannelHandle channel, QVector<MacroAction> actions);

  private:
    std::unique_ptr<MacroRecorder> m_pMacroRecorder;
    std::unique_ptr<MacroDAO> m_pMacroDAO;

    PlayerManager* m_pPlayerManager;
};
