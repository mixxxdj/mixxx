#pragma once

#include <QObject>
#include <QQmlEngine>

#include "rendergraph/node.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

class WaveformWidgetRenderer;

namespace allshaders {
class WaveformRendererEndOfTrack;
class WaveformRendererPreroll;
class WaveformRendererRGB;
class WaveformRenderBeat;
} // namespace allshaders

namespace mixxx {
namespace qml {

class QmlWaveformRendererFactory : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
  public:
    struct Renderer {
        ::WaveformRendererAbstract* renderer{nullptr};
        rendergraph::BaseNode* node{nullptr};
    };

    virtual Renderer create(WaveformWidgetRenderer* waveformWidget) const = 0;
};

class QmlWaveformRendererEndOfTrack
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged REQUIRED)
    QML_NAMED_ELEMENT(WaveformRendererEndOfTrack)

  public:
    QmlWaveformRendererEndOfTrack();

    const QColor& color() const {
        return m_color;
    }
    void setColor(QColor color) {
        m_color = color;
        emit colorChanged(m_color);
    }

    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;

  signals:
    void colorChanged(const QColor&);

  private:
    QColor m_color;
};

class QmlWaveformRendererPreroll
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged REQUIRED)
    QML_NAMED_ELEMENT(WaveformRendererPreroll)

  public:
    QmlWaveformRendererPreroll();

    const QColor& color() const {
        return m_color;
    }
    void setColor(QColor color) {
        m_color = color;
        emit colorChanged(m_color);
    }

    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;
  signals:
    void colorChanged(const QColor&);

  private:
    QColor m_color;
};

class QmlWaveformRendererRGB
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QColor axesColor READ axesColor WRITE setAxesColor NOTIFY axesColorChanged REQUIRED)
    Q_PROPERTY(QColor lowColor READ lowColor WRITE setLowColor NOTIFY lowColorChanged REQUIRED)
    Q_PROPERTY(QColor midColor READ midColor WRITE setMidColor NOTIFY midColorChanged REQUIRED)
    Q_PROPERTY(QColor highColor READ highColor WRITE setHighColor NOTIFY highColorChanged REQUIRED)
    QML_NAMED_ELEMENT(WaveformRendererRGB)

  public:
    QmlWaveformRendererRGB();

    const QColor& axesColor() const {
        return m_axesColor;
    }
    void setAxesColor(QColor color) {
        m_axesColor = color;
        emit axesColorChanged(m_axesColor);
    }

    const QColor& lowColor() const {
        return m_lowColor;
    }
    void setLowColor(QColor color) {
        m_lowColor = color;
        emit lowColorChanged(m_lowColor);
    }

    const QColor& midColor() const {
        return m_lowColor;
    }
    void setMidColor(QColor color) {
        m_midColor = color;
        emit midColorChanged(m_lowColor);
    }

    const QColor& highColor() const {
        return m_lowColor;
    }
    void setHighColor(QColor color) {
        m_highColor = color;
        emit highColorChanged(m_lowColor);
    }

    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;
  signals:
    void axesColorChanged(const QColor&);
    void lowColorChanged(const QColor&);
    void midColorChanged(const QColor&);
    void highColorChanged(const QColor&);

  private:
    QColor m_axesColor;
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;
};

class QmlWaveformRendererBeat
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged REQUIRED)
    QML_NAMED_ELEMENT(WaveformRendererBeat)

  public:
    QmlWaveformRendererBeat();

    const QColor& color() const {
        return m_color;
    }
    void setColor(QColor color) {
        m_color = color;
        emit colorChanged(m_color);
    }

    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;
  signals:
    void colorChanged(const QColor&);

  private:
    QColor m_color;
};

class QmlWaveformMarkRange : public QObject {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(QColor disabledColor READ disabledColor WRITE setDisabledColor)
    Q_PROPERTY(double opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(double disabledOpacity READ disabledOpacity WRITE setDisabledOpacity)
    Q_PROPERTY(QColor durationTextColor READ durationTextColor WRITE setDurationTextColor)
    Q_PROPERTY(QString startControl READ startControl WRITE setStartControl)
    Q_PROPERTY(QString endControl READ endControl WRITE setEndControl)
    Q_PROPERTY(QString enabledControl READ enabledControl WRITE setEnabledControl)
    Q_PROPERTY(QString visibilityControl READ visibilityControl WRITE setVisibilityControl)
    Q_PROPERTY(QString durationTextLocation READ durationTextLocation WRITE setDurationTextLocation)
    QML_NAMED_ELEMENT(WaveformMarkRange)

  public:
    QColor color() const {
        return m_color;
    }

    void setColor(const QColor& value) {
        m_color = value;
    }

    QColor disabledColor() const {
        return m_disabledColor;
    }

    void setDisabledColor(const QColor& value) {
        m_disabledColor = value;
    }

    double opacity() const {
        return m_opacity;
    }

    void setOpacity(double value) {
        m_opacity = value;
    }

    double disabledOpacity() const {
        return m_disabledOpacity;
    }

    void setDisabledOpacity(double value) {
        m_disabledOpacity = value;
    }

    QColor durationTextColor() const {
        return m_durationTextColor;
    }

    void setDurationTextColor(const QColor& value) {
        m_durationTextColor = value;
    }

    QString startControl() const {
        return m_startControl;
    }

    void setStartControl(const QString& value) {
        m_startControl = value;
    }

    QString endControl() const {
        return m_endControl;
    }

    void setEndControl(const QString& value) {
        m_endControl = value;
    }

    QString enabledControl() const {
        return m_enabledControl;
    }

    void setEnabledControl(const QString& value) {
        m_enabledControl = value;
    }

    QString visibilityControl() const {
        return m_visibilityControl;
    }

    void setVisibilityControl(const QString& value) {
        m_visibilityControl = value;
    }

    QString durationTextLocation() const {
        return m_durationTextLocation;
    }

    void setDurationTextLocation(const QString& value) {
        m_durationTextLocation = value;
    }

  private:
    double m_opacity{0.5};
    double m_disabledOpacity{0.5};
    QColor m_color;
    QColor m_disabledColor;
    QColor m_durationTextColor;
    QString m_startControl;
    QString m_endControl;
    QString m_enabledControl;
    QString m_visibilityControl;
    QString m_durationTextLocation;
};

class QmlWaveformMark : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString control READ control WRITE setControl)
    Q_PROPERTY(QString visibilityControl READ visibilityControl WRITE setVisibilityControl)
    Q_PROPERTY(QString color READ color WRITE setColor)
    Q_PROPERTY(QString textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QString align READ align WRITE setAlign)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString pixmap READ pixmap WRITE setPixmap)
    Q_PROPERTY(QString icon READ icon WRITE setIcon)
    QML_NAMED_ELEMENT(WaveformMark)
  public:
    QString control() const {
        return m_control;
    }
    void setControl(const QString& value) {
        m_control = value;
    }
    QString visibilityControl() const {
        return m_visibilityControl;
    }
    void setVisibilityControl(const QString& value) {
        m_visibilityControl = value;
    }
    QString color() const {
        return m_color;
    }
    void setColor(const QString& value) {
        m_color = value;
    }
    QString textColor() const {
        return m_textColor;
    }
    void setTextColor(const QString& value) {
        m_textColor = value;
    }
    QString align() const {
        return m_align;
    }
    void setAlign(const QString& value) {
        m_align = value;
    }
    QString text() const {
        return m_text;
    }
    void setText(const QString& value) {
        m_text = value;
    }
    QString pixmap() const {
        return m_pixmap;
    }
    void setPixmap(const QString& value) {
        m_pixmap = value;
    }
    QString icon() const {
        return m_icon;
    }
    void setIcon(const QString& value) {
        m_icon = value;
    }

  private:
    QString m_control;
    QString m_visibilityControl;
    QString m_color;
    QString m_textColor;
    QString m_align;
    QString m_text;
    QString m_pixmap;
    QString m_icon;
};

class QmlWaveformUntilMark : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool showTime READ showTime WRITE setShowTime)
    Q_PROPERTY(bool showBeats READ showBeats WRITE setShowBeats)
    Q_PROPERTY(HAlignment align READ align WRITE setAlign)
    Q_PROPERTY(int textSize READ textSize WRITE setTextSize)

    QML_NAMED_ELEMENT(WaveformUntilMark)
  public:
    enum HAlignment { AlignTop = Qt::AlignTop,
        AlignCenter = Qt::AlignCenter,
        AlignBottom = Qt::AlignBottom };
    Q_ENUM(HAlignment)

    bool showTime() const {
        return m_showTime;
    }
    void setShowTime(bool showTime) {
        m_showTime = showTime;
    }
    bool showBeats() const {
        return m_showBeats;
    }
    void setShowBeats(bool showBeats) {
        m_showBeats = showBeats;
    }
    HAlignment align() const {
        return m_align;
    }
    void setAlign(HAlignment align) {
        m_align = align;
    }
    int textSize() const {
        return m_textSize;
    }
    void setTextSize(int textSize) {
        m_textSize = textSize;
    }

  private:
    bool m_showTime;
    bool m_showBeats;
    HAlignment m_align;
    int m_textSize;
};

class QmlWaveformRendererMarkRange
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QmlWaveformMarkRange> ranges READ ranges)
    Q_CLASSINFO("DefaultProperty", "ranges")
    QML_NAMED_ELEMENT(WaveformRendererMarkRange)

  public:
    QmlWaveformRendererMarkRange();

    QQmlListProperty<QmlWaveformMarkRange> ranges() {
        return {this, &m_ranges};
    }

    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;

  private:
    QList<QmlWaveformMarkRange*> m_ranges;
};

class QmlWaveformRendererStem
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    QML_NAMED_ELEMENT(WaveformRendererStem)

  public:
    QmlWaveformRendererStem();

#ifdef __STEM__
    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;
#else
    Renderer create(WaveformWidgetRenderer* waveformWidget) const override {
        return Renderer{};
    }

#endif
};

class QmlWaveformRendererMark
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QmlWaveformMark> marks READ marks)
    Q_PROPERTY(QColor playMarkerColor READ playMarkerColor WRITE setPlayMarkerColor)
    Q_PROPERTY(QColor playMarkerBackground READ playMarkerBackground WRITE setPlayMarkerBackground)
    Q_PROPERTY(QmlWaveformMark* defaultMark READ defaultMark WRITE setDefaultMark)
    Q_PROPERTY(QmlWaveformUntilMark* untilMark READ untilMark FINAL)
    Q_CLASSINFO("DefaultProperty", "marks")
    QML_NAMED_ELEMENT(WaveformRendererMark)

  public:
    QmlWaveformRendererMark();

    QmlWaveformMark* defaultMark() const {
        return m_defaultMark;
    }

    QmlWaveformUntilMark* untilMark() const {
        return m_untilMark.get();
    }
    void setDefaultMark(QmlWaveformMark* defaultMark) {
        m_defaultMark = defaultMark;
    }

    const QColor& playMarkerColor() const {
        return m_playMarkerColor;
    }
    void setPlayMarkerColor(const QColor& playMarkerColor) {
        m_playMarkerColor = playMarkerColor;
    }

    const QColor& playMarkerBackground() const {
        return m_playMarkerBackground;
    }
    void setPlayMarkerBackground(const QColor& playMarkerBackground) {
        m_playMarkerBackground = playMarkerBackground;
    }

    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;

    QQmlListProperty<QmlWaveformMark> marks() {
        return {this, &m_marks};
    }

  private:
    QColor m_playMarkerColor;
    QColor m_playMarkerBackground;
    QList<QmlWaveformMark*> m_marks;
    QmlWaveformMark* m_defaultMark;
    std::unique_ptr<QmlWaveformUntilMark> m_untilMark;
};

} // namespace qml
} // namespace mixxx
