#ifndef WAVEFORMRENDERMARK_H
#define WAVEFORMRENDERMARK_H

#include <QObject>

#include "skin/skincontext.h"
#include "util/class.h"
#include "util/math.h"
#include "waveform/renderers/waveformmarkset.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "library/dao/cue.h"
#include "preferences/configobject.h"

class WaveformRenderMark : public QObject, public WaveformRendererAbstract {
    Q_OBJECT
  public:
    explicit WaveformRenderMark(WaveformWidgetRenderer* waveformWidgetRenderer);

    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

    virtual void onResize() override;

    // Called when a new track is loaded.
    virtual void onSetTrack();

  public slots:
    // Called when the loaded track's cues are added, deleted or modified and
    // when a new track is loaded.
    // It updates the marks' names and regenerates their image if needed.
    void slotCuesUpdated();

  private:
    void generateMarkImage(WaveformMark* pMark);
    // algorithm by http://www.nbdtech.com/Blog/archive/2008/04/27/Calculating-the-Perceived-Brightness-of-a-Color.aspx
    // NOTE(Swiftb0y): please suggest if I should you use other methods
    // (like the W3C algorithm) or if this approach is to to performance hungry
    int brightness(QColor& c) {
       return static_cast<int>(sqrtf(
          c.red() * c.red() * .241 +
          c.green() * c.green() * .691 +
          c.blue() * c.blue() * .068));
    };
    WaveformMarkSet m_marks;
    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};

#endif
