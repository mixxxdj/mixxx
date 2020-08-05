#include "macromanager.h"

#include <QDebug>

#include "mixer/basetrackplayer.h"
#include "util/db/dbconnectionpooled.h"

MacroManager::MacroManager(
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        PlayerManager* pPlayerManager)
        : m_pMacroRecorder(std::make_unique<MacroRecorder>()),
          m_pMacroDAO(std::make_unique<MacroDAO>(mixxx::DbConnectionPooled(pDbConnectionPool))),
          m_pPlayerManager(pPlayerManager) {
    connect(getRecorder(),
            &MacroRecorder::saveMacro,
            this,
            &MacroManager::slotSaveMacro);
}

void MacroManager::slotSaveMacro(ChannelHandle channel, QVector<MacroAction> actions) {
    qCDebug(macroLoggingCategory) << "Saving Macro for channel" << channel.handle();
    auto track = m_pPlayerManager->getPlayer(channel)->getLoadedTrack();
    m_pMacroDAO->saveMacro(track->getId(), "Unnamed Macro", actions, Macro::StateFlags::Enabled);
}

MacroRecorder* MacroManager::getRecorder() {
    return m_pMacroRecorder.get();
}

MacroDAO* MacroManager::getDAO() {
    return m_pMacroDAO.get();
}
