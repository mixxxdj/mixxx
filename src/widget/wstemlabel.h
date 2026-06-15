#pragma once

#include "control/controlproxy.h"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "widget/wlabel.h"

class DlgTrackInfo;
class TrackModel;

class WStemLabel : public WLabel {
    Q_OBJECT
  public:
    explicit WStemLabel(QWidget* pParent, UserSettingsPointer pConfig);

    void setup(const QDomNode& node, const SkinContext& context) override;

  public slots:
    void slotTrackLoaded(TrackPointer pTrack);
    void slotTrackUnloaded(TrackPointer pTrack);

  private slots:
    void updateLabel();

  protected:
    void mouseDoubleClickEvent(QMouseEvent*) override;

  private:
    void setTextColor(const QColor& color);
    void setLabelText(const QString& text);

    mixxx::Stem m_stem;
    TrackPointer m_pTrack;
    QString m_group;
    const UserSettingsPointer m_pConfig;
    std::unique_ptr<DlgTrackInfo> m_pDlgTrackInfo;
    int m_stemNo;
};
