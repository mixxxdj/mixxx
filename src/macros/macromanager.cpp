#include "macromanager.h"

#include <QDebug>

#include "mixer/basetrackplayer.h"
#include "util/db/dbconnectionpooled.h"

MacroManager::MacroManager(
        mixxx::DbConnectionPoolPtr pDbConnectionPool)
        : m_pMacroRecorder(std::make_unique<MacroRecorder>()),
          m_pMacroDao(std::make_unique<MacroDAO>()) {
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

void MacroManager::setPlayerManager(PlayerManager* pPlayerManager) {
    m_pPlayerManager = pPlayerManager;
}

void MacroManager::slotSaveMacroFromChannel(QVector<MacroAction> actions, ChannelHandle channel) {
    qCDebug(macroLoggingCategory) << "Saving Macro from channel" << channel.handle();
    slotSaveMacro(actions, m_pPlayerManager->getPlayer(channel)->getLoadedTrack()->getId());
}

void MacroManager::slotSaveMacro(QVector<MacroAction> actions, TrackId trackId) {
    qCDebug(macroLoggingCategory) << "Saving Macro for track" << trackId;
    if (actions.empty()) {
        qCDebug(macroLoggingCategory) << "Macro empty, aborting save!";
    } else {
        m_pMacroDao->saveMacro(
                trackId,
                "Unnamed Macro",
                actions,
                Macro::StateFlag::Enabled);
    }
}

MacroRecorder* MacroManager::getRecorder() {
    return m_pMacroRecorder.get();
}
