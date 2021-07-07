#pragma once

#include "control/quickaction.h"

class QuickActionsManager {
  public:
    static std::shared_ptr<QuickActionsManager> globalInstance() {
        return s_pGlobalInstance;
    };
    static void setGlobalInstance(std::shared_ptr<QuickActionsManager> pQuickActionsManager) {
        s_pGlobalInstance = pQuickActionsManager;
    }

    QuickActionsManager() {
        for (int i = 1; i <= 1; ++i) {
            m_quickActions.push_back(std::make_unique<QuickAction>(i));
        }
    }

    bool recordCOValue(const ConfigKey& key, double value) {
        bool recorded = false;
        for (std::unique_ptr<QuickAction>& quickAction : m_quickActions) {
            if (quickAction->recordCOValue(key, value)) {
                recorded = true;
            }
        }
        return recorded;
    }

  private:
    std::vector<std::unique_ptr<QuickAction>> m_quickActions;

    static std::shared_ptr<QuickActionsManager> s_pGlobalInstance;
};
