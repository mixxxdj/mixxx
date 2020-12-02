#ifndef HSVWAVEFORMWIDGET_H
#define HSVWAVEFORMWIDGET_H

#include <QByteArrayData>
#include <QString>
#include <QWidget>

#include "waveform/widgets/waveformwidgettype.h"
#include "waveformwidgetabstract.h"

class QObject;
class QPaintEvent;

class HSVWaveformWidget : public QWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~HSVWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::HSVWaveform; }

    static inline QString getWaveformWidgetName() { return tr("HSV"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

  private:
    HSVWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // HSVWAVEFORMWIDGET_H
