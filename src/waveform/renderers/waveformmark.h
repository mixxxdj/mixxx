#pragma once
#include <QDomNode>
#include <QHash>
#include <QImage>
#include <atomic>
#include <memory>
#include <optional>
#include <variant>

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "engine/controls/cuecontrol.h"
#include "track/cue.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "waveform/waveformmarklabel.h"

class SkinContext;
class QOpenGLTexture;

class WaveformMark;
typedef QSharedPointer<WaveformMark> WaveformMarkPointer;

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
        Graphics() = default;
        virtual ~Graphics() = default;
        // non-copyable
        Graphics(const Graphics&) = delete;
        Graphics& operator=(const Graphics&) = delete;
        // non-movable
        Graphics(Graphics&&) = delete;
        Graphics& operator=(Graphics&&) = delete;
    };

    WaveformMark(
            const QString& group,
            const QDomNode& node,
            const SkinContext& context,
            int priority,
            const WaveformSignalColors& signalColors,
            int hotCue = Cue::kNoHotCue);

    enum class WaveformMarkConstructionError {
        EndIconInvalidArgumentCount,
        PixmapNotFound,
        EndPixmapNotFound,
        IconNotFound,
        EndIconNotFound,
    };

    // FIXME we are using `std::variant` instead of `std::expected` as Mixxx
    // doesn't yet support C++23.
    static std::variant<WaveformMarkPointer, WaveformMarkConstructionError> create(
            const QString& group,
            const QString& positionControl,
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
        return !m_pGraphics || m_pGraphics->m_obsolete ||
                m_styleGeneration != s_styleGeneration.load();
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

    // Rekordbox-style memory cue markers. When enabled, hotcues from
    // kFirstMemoryCueHotCue on (0-based, i.e. hotcue 9 and up) are drawn as
    // a small triangle in the cue's color instead of a numbered label.
    static constexpr int kFirstMemoryCueHotCue = 8;
    static void setShowMemoryCueMarkers(bool show);
    static bool showMemoryCueMarkers();
    // Vertical placement of the memory cue triangle: Qt::AlignTop,
    // Qt::AlignBottom or both (Qt::AlignTop | Qt::AlignBottom).
    static void setMemoryCueMarkerAlign(Qt::Alignment align);
    static Qt::Alignment memoryCueMarkerAlign();
    // Width of the memory cue marker line in logical pixels; 0 hides the line.
    static constexpr int kMinMemoryCueLineWidth = 0;
    static constexpr int kDefaultMemoryCueLineWidth = 1;
    static constexpr int kMaxMemoryCueLineWidth = 5;
    static void setMemoryCueMarkerLineWidth(int width);
    static int memoryCueMarkerLineWidth();
    // Opacity of the memory cue line and triangles in percent (names keep
    // full opacity, they have their own background options).
    static constexpr int kMinMemoryCueMarkerOpacity = 10;
    static constexpr int kMaxMemoryCueMarkerOpacity = 100;
    static void setMemoryCueMarkerOpacity(int percent);
    static int memoryCueMarkerOpacity();
    // Size (width and height) of the memory cue triangle in the scrolling
    // waveforms, in logical pixels. The overview scales it down.
    static constexpr int kMinMemoryCueMarkerSize = 5;
    static constexpr int kDefaultMemoryCueMarkerSize = 11;
    static constexpr int kMaxMemoryCueMarkerSize = 31;
    static void setMemoryCueMarkerSize(int size);
    static int memoryCueMarkerSize();
    // Optional custom outline color of the memory cue triangle. When off,
    // the outline uses the mark's automatic border color.
    static void setMemoryCueMarkerCustomOutline(bool enabled);
    static bool memoryCueMarkerCustomOutline();
    static void setMemoryCueMarkerOutlineColor(const QColor& color);
    static QColor memoryCueMarkerOutlineColor();
    // Optionally draw all memory cues in one color (e.g. rekordbox red)
    // instead of each cue's own color.
    static void setMemoryCueMarkerSingleColor(bool enabled);
    static bool memoryCueMarkerSingleColor();
    static void setMemoryCueMarkerColor(const QColor& color);
    static QColor memoryCueMarkerColor();
    // Colors to draw this mark as a memory cue with: the single color (and
    // matching contrast colors) when enabled, otherwise the cue's own colors.
    QColor memoryCueColor() const;
    QColor memoryCueBorderColor() const;
    QColor memoryCueLabelColor() const;
    // Outline color to draw this mark's memory cue triangle with.
    QColor memoryCueOutlineColor() const;
    // Show the cue's name (e.g. the rekordbox comment) next to the triangle.
    static void setMemoryCueMarkerShowNames(bool show);
    static bool memoryCueMarkerShowNames();
    // Longest memory cue name drawn, in logical pixels; longer names are elided.
    static constexpr float kMaxMemoryCueNameWidth = 150.f;
    // Vertical placement of the name: Qt::AlignTop or Qt::AlignBottom.
    static void setMemoryCueNameAlign(Qt::Alignment align);
    static Qt::Alignment memoryCueNameAlign();
    // Background box of the name: none, the cue color (semi-transparent) or
    // a custom color (which may include transparency).
    enum class MemoryCueNameBackground {
        Off = 0,
        CueColor = 1,
        CustomColor = 2,
    };
    static void setMemoryCueNameBackground(MemoryCueNameBackground background);
    static MemoryCueNameBackground memoryCueNameBackground();
    static void setMemoryCueNameBackgroundCustomColor(const QColor& color);
    static QColor memoryCueNameBackgroundCustomColor();
    // Optional custom text color of the name.
    static void setMemoryCueNameCustomTextColor(bool enabled);
    static bool memoryCueNameCustomTextColor();
    static void setMemoryCueNameTextColor(const QColor& color);
    static QColor memoryCueNameTextColor();
    // Text color to draw this mark's memory cue name with.
    QColor memoryCueNameEffectiveTextColor() const;
    // Background color of this mark's name box (transparent when off).
    QColor memoryCueNameBackgroundColor() const;
    bool isMemoryCueMarker() const;
    // Where memory cue markers are drawn. In the other view the cues are
    // shown as normal numbered hotcues.
    enum class MemoryCueMarkerView {
        Both = 0,
        ScrollingWaveformsOnly = 1,
        OverviewOnly = 2,
    };
    static void setMemoryCueMarkerView(MemoryCueMarkerView view);
    static MemoryCueMarkerView memoryCueMarkerView();
    // isMemoryCueMarker() and enabled for that view.
    bool isMemoryCueMarkerInScrollingWaveforms() const;
    bool isMemoryCueMarkerInOverview() const;

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
    WaveformMark(
            const QString& group,
            const QString& positionControl,
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

    // Check whether or not there are issues with this marker definition. If
    // there are issues, the first one is returned
    std::optional<WaveformMark::WaveformMarkConstructionError> validate() const;

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
    // Last threshold passed to setBaseColor(), reused for the single memory cue color.
    int m_dimBrightThreshold{127};

    // Bumped whenever a global marker style setting changes, so every mark
    // regenerates its image on the next paint.
    static std::atomic<int> s_styleGeneration;
    static std::atomic<bool> s_showMemoryCueMarkers;
    static std::atomic<int> s_memoryCueMarkerView;
    static std::atomic<int> s_memoryCueMarkerAlign;
    static std::atomic<int> s_memoryCueMarkerLineWidth;
    static std::atomic<int> s_memoryCueMarkerSize;
    static std::atomic<int> s_memoryCueMarkerOpacity;
    static std::atomic<bool> s_memoryCueMarkerCustomOutline;
    static std::atomic<QRgb> s_memoryCueMarkerOutlineColor;
    static std::atomic<bool> s_memoryCueMarkerSingleColor;
    static std::atomic<QRgb> s_memoryCueMarkerColor;
    static std::atomic<bool> s_memoryCueMarkerShowNames;
    static std::atomic<bool> s_memoryCueNameAlignTop;
    static std::atomic<int> s_memoryCueNameBackground;
    static std::atomic<QRgb> s_memoryCueNameBackgroundColor;
    static std::atomic<bool> s_memoryCueNameCustomTextColor;
    static std::atomic<QRgb> s_memoryCueNameTextColor;
    int m_styleGeneration{-1};

    QImage generateMemoryCueImage(float devicePixelRatio);

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
