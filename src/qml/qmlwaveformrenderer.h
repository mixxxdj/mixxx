#pragma once

#include <QObject>
#include <QQmlEngine>

#include "waveform/renderers/allshader/waveformrenderersignalbase.h"
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
        std::unique_ptr<rendergraph::BaseNode> node{nullptr};
    };

    virtual bool isSupported() const {
        return true;
    }

    virtual Renderer create(WaveformWidgetRenderer* waveformWidget) const = 0;
};

class QmlWaveformRendererEndOfTrack
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QColor color MEMBER m_color NOTIFY colorChanged REQUIRED)
    Q_PROPERTY(int endOfTrackWarningTime MEMBER m_endOfTrackWarningTime NOTIFY
                    endOfTrackWarningTimeChanged REQUIRED)
    QML_NAMED_ELEMENT(WaveformRendererEndOfTrack)

  public:
    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;

  signals:
    void colorChanged(const QColor&);
    void endOfTrackWarningTimeChanged(int m_endOfTrackWarningTime);

  private:
    QColor m_color;
    int m_endOfTrackWarningTime;
};

class QmlWaveformRendererPreroll
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QColor color MEMBER m_color NOTIFY colorChanged REQUIRED)
    QML_NAMED_ELEMENT(WaveformRendererPreroll)

  public:
    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;
  signals:
    void colorChanged(const QColor&);

  private:
    QColor m_color;
    ::WaveformRendererAbstract::PositionSource m_position{::WaveformRendererAbstract::Play};
};

class QmlWaveformRendererRGB
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QColor axesColor MEMBER m_axesColor NOTIFY axesColorChanged REQUIRED)
    Q_PROPERTY(QColor lowColor MEMBER m_lowColor NOTIFY lowColorChanged REQUIRED)
    Q_PROPERTY(QColor midColor MEMBER m_midColor NOTIFY midColorChanged REQUIRED)
    Q_PROPERTY(QColor highColor MEMBER m_highColor NOTIFY highColorChanged REQUIRED)
    Q_PROPERTY(double gainAll MEMBER m_gainAll NOTIFY gainAllChanged REQUIRED)
    Q_PROPERTY(double gainLow MEMBER m_gainLow NOTIFY gainLowChanged REQUIRED)
    Q_PROPERTY(double gainMid MEMBER m_gainMid NOTIFY gainMidChanged REQUIRED)
    Q_PROPERTY(double gainHigh MEMBER m_gainHigh NOTIFY gainHighChanged REQUIRED)
    QML_NAMED_ELEMENT(WaveformRendererRGB)

  public:
    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;

  signals:
    void axesColorChanged(const QColor&);
    void lowColorChanged(const QColor&);
    void midColorChanged(const QColor&);
    void highColorChanged(const QColor&);
    void gainAllChanged(double);
    void gainLowChanged(double);
    void gainMidChanged(double);
    void gainHighChanged(double);

  private:
    QColor m_axesColor;
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;

    double m_gainAll;
    double m_gainLow;
    double m_gainMid;
    double m_gainHigh;

    ::WaveformRendererAbstract::PositionSource m_position{::WaveformRendererAbstract::Play};
    allshader::WaveformRendererSignalBase::Options m_options{
            allshader::WaveformRendererSignalBase::Option::None};
};

class QmlWaveformRendererBeat
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QColor color MEMBER m_color NOTIFY colorChanged REQUIRED)
    QML_NAMED_ELEMENT(WaveformRendererBeat)

  public:
    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;
  signals:
    void colorChanged(const QColor&);

  private:
    QColor m_color;
    ::WaveformRendererAbstract::PositionSource m_position{::WaveformRendererAbstract::Play};
};

class QmlWaveformMarkRange : public QObject {
    Q_OBJECT
    Q_PROPERTY(QColor color MEMBER m_color NOTIFY colorChanged)
    Q_PROPERTY(QColor disabledColor MEMBER m_disabledColor NOTIFY disabledColorChanged)
    Q_PROPERTY(double opacity MEMBER m_opacity NOTIFY opacityChanged)
    Q_PROPERTY(double disabledOpacity MEMBER m_disabledOpacity NOTIFY disabledOpacityChanged)
    Q_PROPERTY(QColor durationTextColor MEMBER m_durationTextColor NOTIFY durationTextColorChanged)
    Q_PROPERTY(QString startControl MEMBER m_startControl NOTIFY startControlChanged)
    Q_PROPERTY(QString endControl MEMBER m_endControl NOTIFY endControlChanged)
    Q_PROPERTY(QString enabledControl MEMBER m_enabledControl NOTIFY enabledControlChanged)
    Q_PROPERTY(QString visibilityControl MEMBER m_visibilityControl NOTIFY visibilityControlChanged)
    Q_PROPERTY(QString durationTextLocation MEMBER m_durationTextLocation NOTIFY
                    durationTextLocationChanged)
    QML_NAMED_ELEMENT(WaveformMarkRange)

  public:
    QColor color() const {
        return m_color;
    }

    QColor disabledColor() const {
        return m_disabledColor;
    }

    double opacity() const {
        return m_opacity;
    }

    double disabledOpacity() const {
        return m_disabledOpacity;
    }

    QColor durationTextColor() const {
        return m_durationTextColor;
    }

    QString startControl() const {
        return m_startControl;
    }

    QString endControl() const {
        return m_endControl;
    }

    QString enabledControl() const {
        return m_enabledControl;
    }

    QString visibilityControl() const {
        return m_visibilityControl;
    }

    QString durationTextLocation() const {
        return m_durationTextLocation;
    }

  signals:
    void colorChanged(QColor color);
    void disabledColorChanged(QColor disabledColor);
    void opacityChanged(double opacity);
    void disabledOpacityChanged(double disabledOpacity);
    void durationTextColorChanged(QColor durationTextColor);
    void startControlChanged(QString startControl);
    void endControlChanged(QString endControl);
    void enabledControlChanged(QString enabledControl);
    void visibilityControlChanged(QString visibilityControl);
    void durationTextLocationChanged(QString durationTextLocation);

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
    Q_PROPERTY(QString control MEMBER m_control NOTIFY controlChanged)
    Q_PROPERTY(QString visibilityControl MEMBER m_visibilityControl NOTIFY visibilityControlChanged)
    Q_PROPERTY(QString color MEMBER m_color NOTIFY colorChanged)
    Q_PROPERTY(QString textColor MEMBER m_textColor NOTIFY textColorChanged)
    Q_PROPERTY(QString align MEMBER m_align NOTIFY alignChanged)
    Q_PROPERTY(QString text MEMBER m_text NOTIFY textChanged)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap NOTIFY pixmapChanged)
    Q_PROPERTY(QString icon MEMBER m_icon NOTIFY iconChanged)
    QML_NAMED_ELEMENT(WaveformMark)
  public:
    QString control() const {
        return m_control;
    }
    QString visibilityControl() const {
        return m_visibilityControl;
    }
    QString color() const {
        return m_color;
    }
    QString textColor() const {
        return m_textColor;
    }
    QString align() const {
        return m_align;
    }
    QString text() const {
        return m_text;
    }
    QString pixmap() const {
        return m_pixmap;
    }
    QString icon() const {
        return m_icon;
    }

  signals:
    void controlChanged(QString control);
    void visibilityControlChanged(QString visibilityControl);
    void colorChanged(QString color);
    void textColorChanged(QString textColor);
    void alignChanged(QString align);
    void textChanged(QString text);
    void pixmapChanged(QString pixmap);
    void iconChanged(QString icon);

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
    Q_PROPERTY(bool showTime MEMBER m_showTime NOTIFY showTimeChanged)
    Q_PROPERTY(bool showBeats MEMBER m_showBeats NOTIFY showBeatsChanged)
    Q_PROPERTY(Qt::Alignment align MEMBER m_align NOTIFY alignChanged)
    Q_PROPERTY(int textSize MEMBER m_textSize NOTIFY textSizeChanged)

    QML_NAMED_ELEMENT(WaveformUntilMark)
  public:
    Q_ENUM(Qt::Alignment)

    bool showTime() const {
        return m_showTime;
    }

    bool showBeats() const {
        return m_showBeats;
    }

    Qt::Alignment align() const {
        return m_align;
    }

    int textSize() const {
        return m_textSize;
    }

    float textHeightLimit() const {
        return m_textSize;
    }

  signals:
    void showTimeChanged(bool);
    void showBeatsChanged(bool);
    void alignChanged(Qt::Alignment);
    void textSizeChanged(int);

  private:
    bool m_showTime;
    bool m_showBeats;
    Qt::Alignment m_align;
    int m_textSize;
    int m_textHeightLimit;
};

class QmlWaveformRendererMarkRange
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QmlWaveformMarkRange> ranges READ ranges)
    Q_CLASSINFO("DefaultProperty", "ranges")
    QML_NAMED_ELEMENT(WaveformRendererMarkRange)

  public:
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
    Q_PROPERTY(double gainAll MEMBER m_gainAll NOTIFY gainAllChanged)
    Q_PROPERTY(bool splitStemTracks MEMBER m_splitStemTracks NOTIFY splitStemTracksChanged)
    QML_NAMED_ELEMENT(WaveformRendererStem)

  public:
#ifdef __STEM__
    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;

    bool isSupported() const override {
        return true;
    }
#else
    Renderer create(WaveformWidgetRenderer* waveformWidget) const override {
        return Renderer{};
    }

    bool isSupported() const override {
        return false;
    }
#endif

  signals:
    void gainAllChanged(double);
    void splitStemTracksChanged(bool);

  private:
    double m_gainAll{1.0};
    bool m_splitStemTracks{false};

    ::WaveformRendererAbstract::PositionSource m_position{::WaveformRendererAbstract::Play};
};

class QmlWaveformRendererMark
        : public QmlWaveformRendererFactory {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QmlWaveformMark> marks READ marks)
    Q_PROPERTY(QColor playMarkerColor MEMBER m_playMarkerColor NOTIFY playMarkerColorChanged)
    Q_PROPERTY(QColor playMarkerBackground MEMBER m_playMarkerBackground NOTIFY
                    playMarkerBackgroundChanged)
    Q_PROPERTY(QmlWaveformMark* defaultMark MEMBER m_defaultMark NOTIFY defaultMarkChanged)
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

    Renderer create(WaveformWidgetRenderer* waveformWidget) const override;

    QQmlListProperty<QmlWaveformMark> marks() {
        return {this, &m_marks};
    }
  signals:
    void playMarkerColorChanged(const QColor&);
    void playMarkerBackgroundChanged(const QColor&);
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    void defaultMarkChanged(QmlWaveformMark*);
#else
    void defaultMarkChanged(mixxx::qml::QmlWaveformMark*);
#endif

  private:
    QColor m_playMarkerColor;
    QColor m_playMarkerBackground;
    QList<QmlWaveformMark*> m_marks;
    QmlWaveformMark* m_defaultMark;
    std::unique_ptr<QmlWaveformUntilMark> m_untilMark;
};

} // namespace qml
} // namespace mixxx
