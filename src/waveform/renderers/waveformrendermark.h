#pragma once

#include <QObject>

#include "skin/skincontext.h"
#include "util/class.h"
#include "util/color/color.h"
#include "waveform/renderers/waveformmarkset.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "track/cue.h"
#include "preferences/configobject.h"

class WaveformRenderMark : public QObject, public WaveformRendererAbstract {
    Q_OBJECT

  public:
    explicit WaveformRenderMark(WaveformWidgetRenderer* waveformWidgetRenderer);

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

    void onResize() override;

    // Called when a new track is loaded.
    void onSetTrack() override;

  public slots:
    // Called when the loaded track's cues are added, deleted or modified and
    // when a new track is loaded.
    // It updates the marks' names and regenerates their image if needed.
    // This method is used for hotcues.
    void slotCuesUpdated();

  private:
    void generateMarkImage(WaveformMarkPointer pMark);

    WaveformMarkSet m_marks;
    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};
