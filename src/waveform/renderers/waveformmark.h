#pragma once
#include <QDomNode>
#include <QHash>
#include <QImage>
#include <memory>

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "engine/controls/cuecontrol.h"
#include "track/cue.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "waveform/waveformmarklabel.h"

class SkinContext;
class QOpenGLTexture;

namespace allshader {
class WaveformRenderMark;
} // namespace allshader

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

    WaveformMark(
            const QString& group,
            QString positionControl,
            const QString& visibilityControl,
            const QString& textColor,
            const QString& markAlign,
            const QString& text,
            const QString& pixmapPath,
            const QString& iconPath,
            QColor color,
            int priority,
            int hotCue = Cue::kNoHotCue,
            const WaveformSignalColors& signalColors = {},
            const QString& endPixmapPath = {},
            const QString& endIconPath = {},
            float disabledOpacity = 1.0f,
            float enabledOpacity = 1.0f);
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
    mixxx::CueType getType() const {
        if (!m_typeCO) {
            return mixxx::CueType::Invalid;
        }
        return static_cast<mixxx::CueType>(m_typeCO->get());
    }

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
    template<typename Receiver, typename Slot>
    void connectTypeChanged(Receiver receiver, Slot slot) const {
        if (m_typeCO) {
            m_typeCO->connectValueChanged(receiver, slot, Qt::AutoConnection);
        }
    };
    template<typename Receiver, typename Slot>
    void connectStatusChanged(Receiver receiver, Slot slot) const {
        if (m_statusCO) {
            m_statusCO->connectValueChanged(receiver, slot, Qt::AutoConnection);
        }
    };

    double getSamplePosition() const {
        return m_pPositionCO->get();
    }
    bool isJump() const {
        return m_typeCO &&
                static_cast<mixxx::CueType>(m_typeCO->get()) ==
                mixxx::CueType::Jump;
    }
    bool isLoop() const {
        return m_typeCO &&
                static_cast<mixxx::CueType>(m_typeCO->get()) ==
                mixxx::CueType::Loop;
    }
    bool isStandard() const {
        // A Waveform mark should always have either `isJump`, `isLoop` or
        // `isNormal` returning true!
        return !isLoop() && !isJump();
    }
    double getSampleEndPosition() const {
        if (!m_pEndPositionCO ||
                // A hotcue may have an end position although it isn't a saved
                // loop or jump anymore. This happens when the user changes the cue
                // type. However, we persist the end position if the user wants
                // to restore the cue to a saved loop
                isStandard()) {
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
    // A cue is always considered active if it isn't a saved loop or a saved
    // jump (a.k.a a "standard" cue)
    bool isActive() const {
        return !m_statusCO ||
                static_cast<HotcueControl::Status>(m_statusCO->get()) ==
                HotcueControl::Status::Active;
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

    double opacity() const {
        return isActive() ? m_enabledOpacity : m_disabledOpacity;
    }

    void setNeedsImageUpdate() {
        if (m_pGraphics) {
            m_pGraphics->m_obsolete = true;
        }
        if (m_pEndGraphics) {
            m_pEndGraphics->m_obsolete = true;
        }
    }

    bool needsImageUpdate() const {
        return !m_pGraphics || m_pGraphics->m_obsolete;
    }

    bool needsEndImageUpdate() const {
        return !m_pEndGraphics || m_pEndGraphics->m_obsolete;
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
    QImage generateEndImage(float devicePixelRatio);

    QColor m_textColor;
    QString m_text;
    Qt::Alignment m_align;
    QString m_pixmapPath;
    QString m_endPixmapPath;
    QString m_iconPath;
    QString m_endIconPath;

    double m_enabledOpacity;
    double m_disabledOpacity;

    float m_linePosition;
    float m_offset;
    float m_breadth;

    // When there are overlapping marks, level is increased for each overlapping mark,
    // so that we can draw them at different positions: The marks at the top go lower
    // when the level increased, the marks at the bottom higher.
    int m_level;

    WaveformMarkLabel m_label;

  private:
    QImage performImageGeneration(float devicePixelRatio,
            const QString& pixmapPath,
            const QString& text,
            WaveformMarkLabel* labelMark,
            const QString& iconPath);

    std::unique_ptr<ControlProxy> m_pPositionCO;
    std::unique_ptr<ControlProxy> m_pEndPositionCO;
    std::unique_ptr<ControlProxy> m_pVisibleCO;
    std::unique_ptr<ControlProxy> m_typeCO;
    std::unique_ptr<ControlProxy> m_statusCO;

    std::unique_ptr<Graphics> m_pGraphics;
    std::unique_ptr<Graphics> m_pEndGraphics;

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
