#pragma once

#include <QList>

#include "waveformmark.h"
#include "skin/legacy/skincontext.h"


// This class helps share code between the WaveformRenderMark and WOverview
// constructors and allows to iterate over the orders marks that have to be
// rendered.
class WaveformMarkSet {
  public:
    struct DefaultMarkerStyle {
        QString positionControl;
        QString visibilityControl;
        QString textColor;
        QString markAlign;
        QString text;
        QString pixmapPath;
        QString iconPath;
        QColor color;
    };

    WaveformMarkSet();
    virtual ~WaveformMarkSet();

    void setup(const QString& group, const QDomNode& node,
               const SkinContext& context,
               const WaveformSignalColors& signalColors);

    template<typename Receiver, typename Slot>
    void connectSamplePositionChanged(Receiver receiver, Slot slot) const {
        for (const auto& pMark : std::as_const(m_marks)) {
            if (pMark->isValid()) {
                pMark->connectSamplePositionChanged(receiver, slot);
            }
        }
    };

    template<typename Receiver, typename Slot>
    void connectSampleEndPositionChanged(Receiver receiver, Slot slot) const {
        for (const auto& pMark : std::as_const(m_marks)) {
            if (pMark->isValid()) {
                pMark->connectSampleEndPositionChanged(receiver, slot);
            }
        }
    };

    template<typename Receiver, typename Slot>
    void connectVisibleChanged(Receiver receiver, Slot slot) const {
        for (const auto& pMark : std::as_const(m_marks)) {
            if (pMark->hasVisible()) {
                pMark->connectVisibleChanged(receiver, slot);
            }
        }
    }

    inline QList<WaveformMarkPointer>::const_iterator begin() const {
        return m_marksToRender.begin();
    }
    inline QList<WaveformMarkPointer>::const_iterator end() const {
        return m_marksToRender.end();
    }
    inline QList<WaveformMarkPointer>::const_iterator cbegin() const {
        return m_marksToRender.cbegin();
    }
    inline QList<WaveformMarkPointer>::const_iterator cend() const {
        return m_marksToRender.cend();
    }

    // hotCue must be valid (>= 0 and < kMaxNumberOfHotcues)
    WaveformMarkPointer getHotCueMark(int hotCue) const;
    WaveformMarkPointer getDefaultMark() const;
    WaveformMarkPointer findHoveredMark(QPoint point, Qt::Orientation orientation) const;

    void update();

    void setBreadth(float breadth);

    void clear() {
        m_marks.clear();
        m_marksToRender.clear();
    }

    void addMark(WaveformMarkPointer pMark) {
        m_marks.push_back(pMark);
    }

    void setDefault(const QString& group,
            const DefaultMarkerStyle& model,
            const WaveformSignalColors& signalColors = {});

  private:
    WaveformMarkPointer m_pDefaultMark;
    QList<WaveformMarkPointer> m_marks;
    // List of visible WaveformMarks sorted by the order they appear in the track
    QList<WaveformMarkPointer> m_marksToRender;

    QMap<int, WaveformMarkPointer> m_hotCueMarks;

    DISALLOW_COPY_AND_ASSIGN(WaveformMarkSet);
};
