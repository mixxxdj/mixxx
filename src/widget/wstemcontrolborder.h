#pragma once

#include <QColor>
#include <QString>

#include "control/controlproxy.h"
#include "skin/legacy/skincontext.h"
#include "track/steminfo.h"
#include "track/track.h"

class WStemControlBorder : public WWidgetGroup {
    Q_OBJECT
  public:
    WStemControlBorder(QWidget* pParent);
    ~WStemControlBorder() override = default;

    void setup(const QDomNode& node, const SkinContext& context) override;

    void setStemNumber(int stemNo) {
        m_stemNo = stemNo;
    }

  public slots:
    void slotTrackLoaded(TrackPointer pTrack);
    void slotTrackUnloaded();

  private:
    void updateBorder();
    void setBorderColor(const QColor& color);
    void updateBorderStyle(const QColor& color);

    int m_stemNo{1};
    QString m_group;
    StemInfo m_stemInfo;
};
