#include "macros/macromanager.h"

#include <QDebug>

#include "mixer/basetrackplayer.h"
#include "util/db/dbconnectionpooled.h"

MacroManager::MacroManager()
        : m_pMacroRecorder(std::make_unique<MacroRecorder>()) {
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
    slotSaveMacro(actions, m_pPlayerManager->getPlayer(channel)->getLoadedTrack());
}

void MacroManager::slotSaveMacro(QVector<MacroAction> actions, TrackPointer track) {
    qCDebug(macroLoggingCategory) << "Saving Macro for track" << track->getId();
    if (actions.empty()) {
        qCDebug(macroLoggingCategory) << "Macro empty, aborting save!";
    } else {
        track->addMacro(
                Macro::getFreeSlot(track->getMacros().keys()),
                Macro(actions, "Unnamed Macro", Macro::StateFlag::Enabled));
    }
}

MacroRecorder* MacroManager::getRecorder() {
    return m_pMacroRecorder.get();
}
