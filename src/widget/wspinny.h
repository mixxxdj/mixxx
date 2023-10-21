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
    QImage m_qImage;

    void draw() override;
    void setupVinylSignalQuality() override;
    void updateVinylSignalQualityImage(
            const QColor& qual_color, const unsigned char* data) override;
    void coverChanged() override;
};
