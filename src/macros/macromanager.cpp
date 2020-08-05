#include "macromanager.h"

#include <QDebug>

#include "mixer/basetrackplayer.h"
#include "util/db/dbconnectionpooled.h"

MacroManager::MacroManager(
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        PlayerManager* pPlayerManager)
        : m_pMacroRecorder(std::make_unique<MacroRecorder>()),
          m_pMacroDao(std::make_unique<MacroDAO>()),
          m_pPlayerManager(pPlayerManager) {
    m_pMacroDao->initialize(mixxx::DbConnectionPooled(pDbConnectionPool));
    connect(getRecorder(),
            &MacroRecorder::saveMacro,
            this,
            &MacroManager::slotSaveMacro);
}

void MacroManager::slotSaveMacro(ChannelHandle channel, QVector<MacroAction> actions) {
    qCDebug(macroLoggingCategory) << "Saving Macro for channel" << channel.handle();
    auto track = m_pPlayerManager->getPlayer(channel)->getLoadedTrack();
    m_pMacroDao->saveMacro(track->getId(), "Unnamed Macro", actions, Macro::StateFlags::Enabled);
}

MacroRecorder* MacroManager::getRecorder() {
    return m_pMacroRecorder.get();
}
