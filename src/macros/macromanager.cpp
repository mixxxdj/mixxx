#include "macromanager.h"

MacroManager::MacroManager()
        : m_pMacroRecorder(new MacroRecorder()) {
}

MacroManager::~MacroManager() {
    delete m_pMacroRecorder;
}

MacroRecorder* MacroManager::getRecorder() {
    return m_pMacroRecorder;
}