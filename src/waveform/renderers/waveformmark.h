#ifndef WAVEFORMMARK_H
#define WAVEFORMMARK_H

#include <QDomNode>
#include <QImage>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "track/cue.h"
#include "util/memory.h"
#include "waveform/waveformmarklabel.h"

class SkinContext;
class WaveformSignalColors;

class WOverview;

class WaveformMark {
  public:
    WaveformMark(
            const QString& group,
            const QDomNode& node,
            const SkinContext& context,
            const WaveformSignalColors& signalColors,
            int hotCue = Cue::kNoHotCue);

    // Disable copying
    WaveformMark(const WaveformMark&) = delete;
    WaveformMark& operator=(const WaveformMark&) = delete;

    int getHotCue() const { return m_iHotCue; };

    //The m_pPointCos related function
    bool isValid() const { return m_pPointCos && m_pPointCos->valid(); }

    template <typename Receiver, typename Slot>
    void connectSamplePositionChanged(Receiver receiver, Slot slot) const {
        m_pPointCos->connectValueChanged(receiver, slot, Qt::AutoConnection);
    };
    double getSamplePosition() const { return m_pPointCos->get(); }
    QString getItem() const { return m_pPointCos->getKey().item; }

    // The m_pVisibleCos related function
    bool hasVisible() const { return m_pVisibleCos && m_pVisibleCos->valid(); }
    bool isVisible() const {
        if (!hasVisible()) {
            return true;
        }
        return m_pVisibleCos->get();
    }

    template <typename Receiver, typename Slot>
    void connectVisibleChanged(Receiver receiver, Slot slot) const {
        m_pVisibleCos->connectValueChanged(receiver, slot, Qt::AutoConnection);
    }

    // Sets the appropriate mark colors based on the base color
    void setBaseColor(QColor baseColor, int dimBrightThreshold);
    QColor fillColor() const {
        return m_fillColor;
    }
    QColor borderColor() const {
        return m_borderColor;
    }
    QColor labelColor() const {
        return m_labelColor;
    }

    // Check if a point (in image co-ordinates) lies on drawn image.
    bool contains(QPoint point, Qt::Orientation orientation) const;

    QColor m_textColor;
    QString m_text;
    Qt::Alignment m_align;
    QString m_pixmapPath;

    float m_linePosition;

    WaveformMarkLabel m_label;

  private:
    std::unique_ptr<ControlProxy> m_pPointCos;
    std::unique_ptr<ControlProxy> m_pVisibleCos;
    int m_iHotCue;
    QImage m_image;

    QColor m_fillColor;
    QColor m_borderColor;
    QColor m_labelColor;

    friend class WaveformRenderMark;
};

typedef QSharedPointer<WaveformMark> WaveformMarkPointer;

inline bool operator<(const WaveformMarkPointer& lhs, const WaveformMarkPointer& rhs) {
    double leftPosition = lhs->getSamplePosition();
    int leftHotcue = lhs->getHotCue();
    double rightPosition = rhs->getSamplePosition();
    int rightHotcue = rhs->getHotCue();
    if (leftPosition == rightPosition) {
        // Sort WaveformMarks without hotcues before those with hotcues;
        // if both have hotcues, sort numerically by hotcue number.
        if (leftHotcue == Cue::kNoHotCue && rightHotcue != Cue::kNoHotCue) {
            return true;
        } else if (leftHotcue != Cue::kNoHotCue && rightHotcue == Cue::kNoHotCue) {
            return false;
        } else {
            return leftHotcue < rightHotcue;
        }
    }
    return leftPosition < rightPosition;
}

#endif // WAVEFORMMARK_H
