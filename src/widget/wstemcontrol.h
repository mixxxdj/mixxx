#pragma once

#include <vector>

#include "track/steminfo.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"
#include "waveform/renderers/waveformmark.h"
#include "widget/trackdroptarget.h"
#include "widget/wwidget.h"
#include "widget/wwidgetgroup.h"

class ControlProxy;
class WKnob;
class WStemControl;

class WStemControlBox : public WWidgetGroup {
    Q_OBJECT
  public:
    Q_PROPERTY(bool displayed READ isDisplayed WRITE setDisplayed NOTIFY displayedChanged);

    WStemControlBox(
            const QString& group, QWidget* parent = nullptr);

    void addControl(QWidget* control);
    bool shouldShow() const {
        return m_hasStem && m_displayed;
    }

    bool isDisplayed() const {
        return m_displayed;
    }

    void setDisplayed(bool displayed);
  public slots:
    void slotTrackLoaded(TrackPointer track);

  signals:
    void displayedChanged(bool);

  private:
    std::vector<std::unique_ptr<WStemControl>> m_stemControl;
    QString m_group;
    bool m_hasStem;
    bool m_displayed;
};

class WStemControl : public WWidget {
    Q_OBJECT
  public:
    WStemControl(QWidget* widgetGroup, QWidget* parent, const QString& group, int stemIdx);
    void setStem(const StemInfo& stemInfo);

  protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;

  private:
    QWidget* m_widget;
    QColor m_stemColor;
    std::unique_ptr<ControlProxy> m_mutedCo;

    void updateStyle();
};
