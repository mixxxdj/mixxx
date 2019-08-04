#ifndef SOFTWAREWAVEFORMWIDGET_H
#define SOFTWAREWAVEFORMWIDGET_H

#include <QWidget>

#include "waveformwidgetabstract.h"

class SoftwareWaveformWidget : public QWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~SoftwareWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::SoftwareWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

  private:
    SoftwareWaveformWidget(const char* group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // SOFTWAREWAVEFORMWIDGET_H
