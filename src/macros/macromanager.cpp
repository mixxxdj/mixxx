#include "macromanager.h"

MacroManager::MacroManager()
        : m_pMacroRecorder(std::make_unique<MacroRecorder>()) {
}

MacroRecorder* MacroManager::getRecorder() {
    return m_pMacroRecorder.get();
}