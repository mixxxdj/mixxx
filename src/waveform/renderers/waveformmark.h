#ifndef WAVEFORMMARK_H
#define WAVEFORMMARK_H

#include <QDomNode>
#include <QImage>

#include "control/controlproxy.h"
#include "util/memory.h"

#include "waveform/renderers/waveformmarkproperties.h"
#include "control/controlobject.h"

class SkinContext;
class WaveformSignalColors;

class WOverview;

class WaveformMark {
  public:
    static const int kDefaultHotCue = -1;
    WaveformMark(
            const QString& group,
            const QDomNode& node,
            const SkinContext& context,
            const WaveformSignalColors& signalColors,
            int hotCue = kDefaultHotCue,
            QString item = "");


    // Disable copying
    WaveformMark(const WaveformMark&) = delete;
    WaveformMark& operator=(const WaveformMark&) = delete;

    const WaveformMarkProperties& getProperties() const {
        return m_properties;
    };
    void setProperties(const WaveformMarkProperties& properties) {
        m_properties = properties;
    };

    int getHotCue() const { return m_iHotCue; };
    void setHotCue(int hotCue) { m_iHotCue = hotCue; };

    //The m_pPointCos related function
    bool isValid() const { return m_pPointCos && m_pPointCos->valid(); }
    void connectSamplePositionChanged(const QObject *, const char *) const;
    double getSamplePosition() const { return m_pPointCos->get(); }
    void setHotcueNumber(int i) {
        m_properties.m_text = m_properties.m_text.arg(i);
    }
    

  private:
    std::unique_ptr<ControlProxy> m_pPointCos;
    WaveformMarkProperties m_properties;
    int m_iHotCue;
    QImage m_image;

    friend class WaveformRenderMark;
};

typedef QSharedPointer<WaveformMark> WaveformMarkPointer;

#endif // WAVEFORMMARK_H
