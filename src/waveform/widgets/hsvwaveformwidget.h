#ifndef HSVWAVEFORMWIDGET_H
#define HSVWAVEFORMWIDGET_H

#include <QWidget>

#include "waveformwidgetabstract.h"

class HSVWaveformWidget : public QWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~HSVWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::HSVWaveform; }

    static inline QString getWaveformWidgetName() { return tr("HSV"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

  private:
    HSVWaveformWidget(const char* group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // HSVWAVEFORMWIDGET_H
