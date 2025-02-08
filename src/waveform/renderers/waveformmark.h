#pragma once
#include <QDomNode>
#include <QImage>
#include <memory>

#include "control/controlproxy.h"
#include "track/cue.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "waveform/waveformmarklabel.h"

class SkinContext;
class QOpenGLTexture;

namespace allshader {
class WaveformRenderMark;
}

class WaveformMark {
  public:
    class Graphics {
      public:
        // To indicate that the image for the mark needs to be regenerated,
        // when the text, color, breadth or level are changed.
        bool m_obsolete{};
    };

    WaveformMark(
            const QString& group,
            const QDomNode& node,
            const SkinContext& context,
            int priority,
            const WaveformSignalColors& signalColors,
            int hotCue = Cue::kNoHotCue);
    ~WaveformMark();

    // Disable copying
    WaveformMark(const WaveformMark&) = delete;
    WaveformMark& operator=(const WaveformMark&) = delete;

    float getOffset() const {
        return m_offset;
    }

    int getHotCue() const {
        return m_iHotCue;
    };
    int getPriority() const {
        return m_iPriority;
    };

    // The m_pPositionCO related function
    bool isValid() const {
        return m_pPositionCO && m_pPositionCO->valid();
    }

    template<typename Receiver, typename Slot>
    void connectSamplePositionChanged(Receiver receiver, Slot slot) const {
        m_pPositionCO->connectValueChanged(receiver, slot, Qt::AutoConnection);
    };
    template<typename Receiver, typename Slot>
    void connectSampleEndPositionChanged(Receiver receiver, Slot slot) const {
        if (m_pEndPositionCO) {
            m_pEndPositionCO->connectValueChanged(receiver, slot, Qt::AutoConnection);
        }
    };
    double getSamplePosition() const {
        return m_pPositionCO->get();
    }
    double getSampleEndPosition() const {
        if (!m_pEndPositionCO ||
                // A hotcue may have an end position although it isn't a saved
                // loop anymore. This happens when the user changes the cue
                // type. However, we persist the end position if the user wants
                // to restore the cue to a saved loop
                (m_pTypeCO &&
                        static_cast<mixxx::CueType>(m_pTypeCO->get()) !=
                                mixxx::CueType::Loop)) {
            return Cue::kNoPosition;
        }
        return m_pEndPositionCO->get();
    }
    QString getItem() const {
        return m_pPositionCO->getKey().item;
    }

    // The m_pVisibleCO related function
    bool hasVisible() const {
        return m_pVisibleCO && m_pVisibleCO->valid();
    }
    bool isVisible() const {
        if (!hasVisible()) {
            return true;
        }
        return m_pVisibleCO->toBool();
    }
    bool isShowUntilNext() const {
        return m_showUntilNext;
    }

    template<typename Receiver, typename Slot>
    void connectVisibleChanged(Receiver receiver, Slot slot) const {
        m_pVisibleCO->connectValueChanged(receiver, slot, Qt::AutoConnection);
    }

    void setText(const QString& text) {
        if (m_text != text) {
            m_text = text;
            setNeedsImageUpdate();
        }
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

    void setNeedsImageUpdate() {
        if (m_pGraphics) {
            m_pGraphics->m_obsolete = true;
        }
    }

    bool needsImageUpdate() const {
        return !m_pGraphics || m_pGraphics->m_obsolete;
    }

    void setBreadth(float breadth) {
        if (m_breadth != breadth) {
            m_breadth = breadth;
            setNeedsImageUpdate();
        }
    }

    void setLevel(int level) {
        if (m_level != level) {
            m_level = level;
            setNeedsImageUpdate();
        }
    }

    // Check if a point (in image coordinates) lies on the line
    bool lineHovered(QPoint point, Qt::Orientation orientation) const;
    // Check if a point (in image coordinates) lies on drawn image.
    bool contains(QPoint point, Qt::Orientation orientation) const;

    QImage generateImage(float devicePixelRatio);

    QColor m_textColor;
    QString m_text;
    Qt::Alignment m_align;
    QString m_pixmapPath;
    QString m_iconPath;

    float m_linePosition;
    float m_offset;
    float m_breadth;

    // When there are overlapping marks, level is increased for each overlapping mark,
    // so that we can draw them at different positions: The marks at the top go lower
    // when the level increased, the marks at the bottom higher.
    int m_level;

    WaveformMarkLabel m_label;

  private:
    std::unique_ptr<ControlProxy> m_pPositionCO;
    std::unique_ptr<ControlProxy> m_pEndPositionCO;
    std::unique_ptr<ControlProxy> m_pTypeCO;
    std::unique_ptr<ControlProxy> m_pVisibleCO;

    std::unique_ptr<Graphics> m_pGraphics;

    int m_iPriority;
    int m_iHotCue;

    // Whether this marker is used in the show beats/time until next marker display.
    bool m_showUntilNext;

    QColor m_fillColor;
    QColor m_borderColor;
    QColor m_labelColor;

    friend class WaveformRenderMark;
    friend class WaveformRenderMarkBase;
    friend class allshader::WaveformRenderMark;
};

typedef QSharedPointer<WaveformMark> WaveformMarkPointer;

// This class provides an immutable sortkey for the WaveformMark using sample
// position and hotcue number. IMPORTANT: The Mark's position may be changed after
// a key's creation, and those updates will not be reflected in these sortkeys.
// Currently they are used to render marks on the Overview, a situation where
// temporarily incorrect sort order is acceptable.
class WaveformMarkSortKey {
  public:
    WaveformMarkSortKey(double samplePosition, int priority)
            : m_samplePosition(samplePosition),
              m_priority(priority) {
    }

    bool operator<(const WaveformMarkSortKey& other) const {
        return m_samplePosition == other.m_samplePosition
                ? m_priority < other.m_priority
                : m_samplePosition < other.m_samplePosition;
    }

  private:
    double m_samplePosition;
    int m_priority;
};
