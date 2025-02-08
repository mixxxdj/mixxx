#pragma once

#include <QObject>

#include "skin/legacy/skincontext.h"
#include "util/class.h"
#include "waveform/renderers/waveformmarkset.h"
#include "waveform/renderers/waveformrendererabstract.h"

class WaveformRenderMarkBase : public QObject, public WaveformRendererAbstract {
    Q_OBJECT

  public:
    explicit WaveformRenderMarkBase(
            WaveformWidgetRenderer* waveformWidgetRenderer,
            bool updateImagesImmediately);

    void setup(const QDomNode& node, const SkinContext& context) override;

    bool init() override;

    // Called when a new track is loaded.
    void onSetTrack() override;

    void onResize() override;

  public slots:
    // Called when the loaded track's cues are added, deleted or modified and
    // when a new track is loaded.
    // It updates the marks' names and flags the need for an image update.
    // This method is used for hotcues.
    void slotCuesUpdated();

  private slots:
    // Called when a mark position or visibility changes
    void onMarkChanged(double v);

  protected:
    WaveformMarkSet m_marks;

    void updateMarkImages();

  private:
    const bool m_updateImagesImmediately;

    void updateMarksFromCues();
    void updateMarks();

  private:
    virtual void updateMarkImage(WaveformMarkPointer pMark) = 0;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMarkBase);
};
