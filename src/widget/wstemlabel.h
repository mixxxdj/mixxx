#pragma once

#include "control/controlproxy.h"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "widget/wlabel.h"

class WStemLabel : public WLabel {
    Q_OBJECT
  public:
    explicit WStemLabel(QWidget* pParent = nullptr);

    void setup(const QDomNode& node, const SkinContext& context) override;

  public slots:
    void slotTrackLoaded(TrackPointer pTrack);
    void slotTrackUnloaded(TrackPointer pTrack);

  private slots:
    void updateLabel();

  private:
    void setTextColor(const QColor& color);
    void setLabelText(const QString& text);

    StemInfo m_stemInfo;
    QString m_group;
    int m_stemNo;
};
