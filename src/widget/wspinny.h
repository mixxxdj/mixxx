#pragma once

#include "widget/wspinnybase.h"

class WSpinny : public WSpinnyBase {
    Q_OBJECT
  public:
    WSpinny(QWidget* parent,
            const QString& group,
            UserSettingsPointer pConfig,
            VinylControlManager* pVCMan,
            BaseTrackPlayer* pPlayer);

  private:
    void draw() override;
    void coverChanged() override;
};
