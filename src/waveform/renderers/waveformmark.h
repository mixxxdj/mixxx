#ifndef WAVEFORMMARK_H
#define WAVEFORMMARK_H

#include <QDomNode>
#include <QImage>

#include "control/controlproxy.h"
#include "util/memory.h"

#include "waveform/renderers/waveformmarkproperties.h"

class SkinContext;
class WaveformSignalColors;

class WaveformMark {
  public:
    static const int kDefaultHotCue = -1;

    explicit WaveformMark(int hotCue = kDefaultHotCue);

    void setup(const QString& group, const QDomNode& node,
               const SkinContext& context,
               const WaveformSignalColors& signalColors);

    std::unique_ptr<ControlProxy> m_pPointCos;

    const WaveformMarkProperties& getProperties() const { return m_properties; };
    void setProperties(const WaveformMarkProperties& properties) { m_properties = properties; };

    int getHotCue() const { return m_iHotCue; };
    void setHotCue(int hotCue) { m_iHotCue = hotCue; };

  private:
    WaveformMarkProperties m_properties;
    int m_iHotCue;
    QImage m_image;

    friend class WaveformRenderMark;
};

typedef QSharedPointer<WaveformMark> WaveformMarkPointer;

#endif // WAVEFORMMARK_H
