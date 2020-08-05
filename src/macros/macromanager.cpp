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
            &MacroRecorder::saveMacroFromChannel,
            this,
            &MacroManager::slotSaveMacroFromChannel);
    connect(getRecorder(),
            &MacroRecorder::saveMacro,
            this,
            &MacroManager::slotSaveMacro);
}

void MacroManager::slotSaveMacroFromChannel(QVector<MacroAction> actions, ChannelHandle channel) {
    qCDebug(macroLoggingCategory) << "Saving Macro from channel" << channel.handle();
    slotSaveMacro(actions, m_pPlayerManager->getPlayer(channel)->getLoadedTrack());
}

void MacroManager::slotSaveMacro(QVector<MacroAction> actions, TrackPointer pTrack) {
    qCDebug(macroLoggingCategory) << "Saving Macro for track" << pTrack->getId();
    if (actions.empty()) {
        qCDebug(macroLoggingCategory) << "Macro empty, aborting save!";
    } else {
        m_pMacroDao->saveMacro(
                pTrack->getId(),
                "Unnamed Macro",
                actions,
                Macro::StateFlag::Enabled);
    }
}

MacroRecorder* MacroManager::getRecorder() {
    return m_pMacroRecorder.get();
}
