#include "waveformmark.h"

#include <QOpenGLTexture>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QStringLiteral>
#include <QSvgRenderer>
#include <QtDebug>
#include <algorithm>
#include <memory>
#include <vector>

#include "skin/legacy/skincontext.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "widget/wimagestore.h"
#include "widget/wskincolor.h"

namespace {

// Without some padding, the user would only have a single pixel width that
// would count as hovering over the WaveformMark.
constexpr float lineHoverPadding = 5.0;

Qt::Alignment decodeAlignmentFlags(const QString& alignString, Qt::Alignment defaultFlags) {
    const QStringList stringFlags = alignString.toLower()
                                            .split('|',
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                                                    Qt::SkipEmptyParts);
#else
                                                    QString::SkipEmptyParts);
#endif

    Qt::Alignment hflags;
    Qt::Alignment vflags;

    for (const auto& stringFlag : stringFlags) {
        if (stringFlag == "center") {
            hflags |= Qt::AlignHCenter;
            vflags |= Qt::AlignVCenter;
        } else if (stringFlag == "left") {
            hflags |= Qt::AlignLeft;
        } else if (stringFlag == "hcenter") {
            hflags |= Qt::AlignHCenter;
        } else if (stringFlag == "right") {
            hflags |= Qt::AlignRight;
        } else if (stringFlag == "top") {
            vflags |= Qt::AlignTop;
        } else if (stringFlag == "vcenter") {
            vflags |= Qt::AlignVCenter;
        } else if (stringFlag == "bottom") {
            vflags |= Qt::AlignBottom;
        }
    }

    if (hflags != Qt::AlignLeft && hflags != Qt::AlignHCenter && hflags != Qt::AlignRight) {
        hflags = defaultFlags & Qt::AlignHorizontal_Mask;
    }

    if (vflags != Qt::AlignTop && vflags != Qt::AlignVCenter && vflags != Qt::AlignBottom) {
        vflags = defaultFlags & Qt::AlignVertical_Mask;
    }

    return hflags | vflags;
}

float overlappingMarkerIncrement(const float labelRectHeight, const float breadth) {
    // gradually "compact" the markers if the waveform height is
    // reduced, to avoid multiple markers obscuring the waveform.
    const float threshold = 90.f; // above this, the full increment is used
    const float fullIncrement = labelRectHeight + 2.f;
    const float minIncrement = 2.f; // increment when most compacted

    return std::max(minIncrement, fullIncrement - std::max(0.f, threshold - breadth));
}

#define FOO

bool isShowUntilNextPositionControl(const QString& positionControl) {
    // To identify which markers are included in the beat/time until next marker
    // display, in addition to the hotcues
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    using namespace Qt::Literals::StringLiterals;
    constexpr std::array list = {"cue_point"_L1,
            "intro_start_position"_L1,
            "intro_end_position"_L1,
            "outro_start_position"_L1,
            "outro_end_position"_L1};
#else
    const std::array list = {QLatin1String{"cue_point"},
            QLatin1String{"intro_start_position"},
            QLatin1String{"intro_end_position"},
            QLatin1String{"outro_start_position"},
            QLatin1String{"outro_end_position"}};
#endif
    return std::any_of(list.cbegin(), list.cend(), [positionControl](auto& view) {
        return view == positionControl;
    });
}

} // anonymous namespace

WaveformMark::WaveformMark(
        const QString& group,
        const QString& aPositionControl,
        const QString& visibilityControl,
        const QString& textColor,
        const QString& markAlign,
        const QString& text,
        const QString& pixmapPath,
        const QString& iconPath,
        QColor color,
        int priority,
        int hotCue,
        const WaveformSignalColors& signalColors,
        const QString& endPixmapPath,
        const QString& endIconPath,
        float disabledOpacity,
        float enabledOpacity)
        : m_textColor(textColor),
          m_pixmapPath(pixmapPath),
          m_endPixmapPath(endPixmapPath),
          m_iconPath(iconPath),
          m_endIconPath(endIconPath),
          m_enabledOpacity(enabledOpacity),
          m_disabledOpacity(disabledOpacity),
          m_linePosition{},
          m_breadth{},
          m_level{},
          m_typeCO{},
          m_statusCO{},
          m_iPriority(priority),
          m_iHotCue(hotCue),
          m_showUntilNext{} {
    QString positionControl = aPositionControl;
    QString endPositionControl;
    QString typeControl;
    QString statusControl;
    if (hotCue != Cue::kNoHotCue) {
        QString hotcueNumber = QString::number(hotCue + 1);
        positionControl = QStringLiteral("hotcue_%1_position").arg(hotcueNumber);
        endPositionControl = QStringLiteral("hotcue_%1_endposition").arg(hotcueNumber);
        statusControl = QStringLiteral("hotcue_%1_status").arg(hotcueNumber);
        typeControl = QStringLiteral("hotcue_%1_type").arg(hotcueNumber);
        m_showUntilNext = true;
    } else {
        m_showUntilNext = isShowUntilNextPositionControl(positionControl);
    }

    if (!positionControl.isEmpty() && !group.isEmpty()) {
        m_pPositionCO = std::make_unique<ControlProxy>(group, positionControl);
    }
    if (!endPositionControl.isEmpty() && !group.isEmpty()) {
        m_pEndPositionCO = std::make_unique<ControlProxy>(group, endPositionControl);
        m_statusCO = std::make_unique<ControlProxy>(group, statusControl);
        m_typeCO = std::make_unique<ControlProxy>(group, typeControl);
    }

    if (!visibilityControl.isEmpty() && !group.isEmpty()) {
        ConfigKey key = ConfigKey::parseCommaSeparated(visibilityControl);
        m_pVisibleCO = std::make_unique<ControlProxy>(key);
    }

    if (!color.isValid()) {
        // As a fallback, grab the color from the parent's AxesColor
        color = signalColors.getAxesColor();
        qDebug() << "Didn't get mark <Color>:" << color;
    } else {
        color = WSkinColor::getCorrectColor(color);
    }
    int dimBrightThreshold = signalColors.getDimBrightThreshold();
    setBaseColor(color, dimBrightThreshold);

    if (!m_textColor.isValid()) {
        // Read the text color, otherwise use the parent's BgColor.
        m_textColor = signalColors.getBgColor();
        qDebug() << "Didn't get mark <TextColor>, using parent's <BgColor>:" << m_textColor;
    }

    m_align = decodeAlignmentFlags(markAlign, Qt::AlignBottom | Qt::AlignHCenter);

    // Hotcue text is set by the cue's label in the database, not by the skin.
    if (hotCue == Cue::kNoHotCue) {
        m_text = text;
    }
}

WaveformMark::WaveformMark(const QString& group,
        const QDomNode& node,
        const SkinContext& context,
        int priority,
        const WaveformSignalColors& signalColors,
        int hotCue)
        : m_linePosition{},
          m_offset{},
          m_breadth{},
          m_level{},
          m_iPriority(priority),
          m_iHotCue(hotCue),
          m_showUntilNext{} {
    QString positionControl;
    QString endPositionControl;
    QString typeControl;
    QString statusControl;
    if (hotCue != Cue::kNoHotCue) {
        positionControl = "hotcue_" + QString::number(hotCue + 1) + "_position";
        endPositionControl = "hotcue_" + QString::number(hotCue + 1) + "_endposition";
        typeControl = "hotcue_" + QString::number(hotCue + 1) + "_type";
        statusControl = "hotcue_" + QString::number(hotCue + 1) + "_status";
        m_showUntilNext = true;
    } else {
        positionControl = context.selectString(node, "Control");
        m_showUntilNext = isShowUntilNextPositionControl(positionControl);
    }

    if (!positionControl.isEmpty()) {
        m_pPositionCO = std::make_unique<ControlProxy>(group, positionControl);
    }
    if (!endPositionControl.isEmpty()) {
        m_pEndPositionCO = std::make_unique<ControlProxy>(group, endPositionControl);
        m_typeCO = std::make_unique<ControlProxy>(group, typeControl);
        m_statusCO = std::make_unique<ControlProxy>(group, statusControl);
    }

    QString visibilityControl = context.selectString(node, "VisibilityControl");
    if (!visibilityControl.isEmpty()) {
        ConfigKey key = ConfigKey::parseCommaSeparated(visibilityControl);
        m_pVisibleCO = std::make_unique<ControlProxy>(key);
    }

    QColor color(context.selectString(node, "Color"));
    if (!color.isValid()) {
        // As a fallback, grab the color from the parent's AxesColor
        color = signalColors.getAxesColor();
        qDebug() << "Didn't get mark <Color>, using parent's <AxesColor>:" << color;
    } else {
        color = WSkinColor::getCorrectColor(color);
    }
    int dimBrightThreshold = signalColors.getDimBrightThreshold();
    setBaseColor(color, dimBrightThreshold);

    m_textColor = context.selectString(node, "TextColor");
    if (!m_textColor.isValid()) {
        // Read the text color, otherwise use the parent's BgColor.
        m_textColor = signalColors.getBgColor();
        qDebug() << "Didn't get mark <TextColor>, using parent's <BgColor>:" << m_textColor;
    }

    QString markAlign = context.selectString(node, "Align");
    m_align = decodeAlignmentFlags(markAlign, Qt::AlignBottom | Qt::AlignHCenter);

    // Hotcue text is set by the cue's label in the database, not by the skin.
    if (hotCue == Cue::kNoHotCue) {
        m_text = context.selectString(node, "Text");
    }

    m_pixmapPath = context.selectString(node, "Pixmap");
    if (!m_pixmapPath.isEmpty()) {
        m_pixmapPath = context.makeSkinPath(m_pixmapPath);
    }

    m_endPixmapPath = context.selectString(node, "EndPixmap");
    if (!m_endPixmapPath.isEmpty()) {
        m_endPixmapPath = context.makeSkinPath(m_endPixmapPath);
    }

    m_iconPath = context.selectString(node, "Icon");
    if (!m_iconPath.isEmpty()) {
        m_iconPath = context.makeSkinPath(m_iconPath);
    }

    m_endIconPath = context.selectString(node, "EndIcon");
    if (!m_endIconPath.isEmpty()) {
        m_endIconPath = context.makeSkinPath(m_endIconPath);
    }

    m_enabledOpacity = context.selectDouble(node, "EnabledOpacity", 1);
    m_disabledOpacity = context.selectDouble(node, "DisabledOpacity", 0.5);
}

std::variant<WaveformMarkPointer, WaveformMark::WaveformMarkConstructionError> WaveformMark::create(
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
        int hotCue,
        const WaveformSignalColors& signalColors,
        const QString& endPixmapPath,
        const QString& endIconPath,
        float disabledOpacity,
        float enabledOpacity) {
    // Used WaveformMark's ctor variant is private so we must explicitly call
    // new before wrapping it in a smart pointer
    WaveformMarkPointer pMark(new WaveformMark(group,
            positionControl,
            visibilityControl,
            textColor,
            markAlign,
            text,
            pixmapPath,
            iconPath,
            color,
            priority,
            hotCue,
            signalColors,
            endPixmapPath,
            endIconPath,
            disabledOpacity,
            enabledOpacity));
    if (auto err = pMark->validate(); err.has_value()) {
        return err.value();
    }
    return pMark;
}

WaveformMark::~WaveformMark() = default;

std::atomic<int> WaveformMark::s_styleGeneration{0};
std::atomic<bool> WaveformMark::s_showMemoryCueMarkers{true};
std::atomic<int> WaveformMark::s_memoryCueMarkerView{static_cast<int>(MemoryCueMarkerView::Both)};
std::atomic<int> WaveformMark::s_memoryCueMarkerAlign{static_cast<int>(Qt::AlignBottom)};
std::atomic<int> WaveformMark::s_memoryCueMarkerLineWidth{kDefaultMemoryCueLineWidth};
std::atomic<int> WaveformMark::s_memoryCueMarkerSize{kDefaultMemoryCueMarkerSize};
std::atomic<int> WaveformMark::s_memoryCueMarkerOpacity{kMaxMemoryCueMarkerOpacity};
std::atomic<bool> WaveformMark::s_memoryCueMarkerCustomOutline{false};
std::atomic<QRgb> WaveformMark::s_memoryCueMarkerOutlineColor{qRgb(255, 255, 255)};
std::atomic<bool> WaveformMark::s_memoryCueMarkerSingleColor{false};
std::atomic<QRgb> WaveformMark::s_memoryCueMarkerColor{qRgb(230, 40, 40)};
std::atomic<bool> WaveformMark::s_memoryCueMarkerShowNames{false};
std::atomic<bool> WaveformMark::s_memoryCueNameAlignTop{true};
std::atomic<int> WaveformMark::s_memoryCueNameBackground{
        static_cast<int>(MemoryCueNameBackground::CueColor)};
std::atomic<QRgb> WaveformMark::s_memoryCueNameBackgroundColor{qRgba(0, 0, 0, 153)};
std::atomic<bool> WaveformMark::s_memoryCueNameCustomTextColor{false};
std::atomic<QRgb> WaveformMark::s_memoryCueNameTextColor{qRgb(255, 255, 255)};

// static
void WaveformMark::setShowMemoryCueMarkers(bool show) {
    if (s_showMemoryCueMarkers.exchange(show) != show) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
bool WaveformMark::showMemoryCueMarkers() {
    return s_showMemoryCueMarkers.load();
}

// static
void WaveformMark::setMemoryCueMarkerAlign(Qt::Alignment align) {
    int value = 0;
    if (align.testFlag(Qt::AlignTop)) {
        value |= static_cast<int>(Qt::AlignTop);
    }
    if (align.testFlag(Qt::AlignBottom) || value == 0) {
        value |= static_cast<int>(Qt::AlignBottom);
    }
    if (s_memoryCueMarkerAlign.exchange(value) != value) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
Qt::Alignment WaveformMark::memoryCueMarkerAlign() {
    const int value = s_memoryCueMarkerAlign.load();
    Qt::Alignment align;
    if (value & static_cast<int>(Qt::AlignTop)) {
        align |= Qt::AlignTop;
    }
    if (value & static_cast<int>(Qt::AlignBottom)) {
        align |= Qt::AlignBottom;
    }
    return align;
}

// static
void WaveformMark::setMemoryCueMarkerLineWidth(int width) {
    width = std::clamp(width, kMinMemoryCueLineWidth, kMaxMemoryCueLineWidth);
    if (s_memoryCueMarkerLineWidth.exchange(width) != width) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
int WaveformMark::memoryCueMarkerLineWidth() {
    return s_memoryCueMarkerLineWidth.load();
}

// static
void WaveformMark::setMemoryCueMarkerOpacity(int percent) {
    percent = std::clamp(percent, kMinMemoryCueMarkerOpacity, kMaxMemoryCueMarkerOpacity);
    if (s_memoryCueMarkerOpacity.exchange(percent) != percent) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
int WaveformMark::memoryCueMarkerOpacity() {
    return s_memoryCueMarkerOpacity.load();
}

// static
void WaveformMark::setMemoryCueMarkerSize(int size) {
    size = std::clamp(size, kMinMemoryCueMarkerSize, kMaxMemoryCueMarkerSize);
    if (s_memoryCueMarkerSize.exchange(size) != size) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
int WaveformMark::memoryCueMarkerSize() {
    return s_memoryCueMarkerSize.load();
}

// static
void WaveformMark::setMemoryCueMarkerCustomOutline(bool enabled) {
    if (s_memoryCueMarkerCustomOutline.exchange(enabled) != enabled) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
bool WaveformMark::memoryCueMarkerCustomOutline() {
    return s_memoryCueMarkerCustomOutline.load();
}

// static
void WaveformMark::setMemoryCueMarkerOutlineColor(const QColor& color) {
    const QRgb rgb = color.isValid() ? color.rgb() : qRgb(255, 255, 255);
    if (s_memoryCueMarkerOutlineColor.exchange(rgb) != rgb) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
QColor WaveformMark::memoryCueMarkerOutlineColor() {
    return QColor::fromRgb(s_memoryCueMarkerOutlineColor.load());
}

// static
void WaveformMark::setMemoryCueMarkerSingleColor(bool enabled) {
    if (s_memoryCueMarkerSingleColor.exchange(enabled) != enabled) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
bool WaveformMark::memoryCueMarkerSingleColor() {
    return s_memoryCueMarkerSingleColor.load();
}

// static
void WaveformMark::setMemoryCueMarkerColor(const QColor& color) {
    const QRgb rgb = color.isValid() ? color.rgb() : qRgb(230, 40, 40);
    if (s_memoryCueMarkerColor.exchange(rgb) != rgb) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
QColor WaveformMark::memoryCueMarkerColor() {
    return QColor::fromRgb(s_memoryCueMarkerColor.load());
}

QColor WaveformMark::memoryCueColor() const {
    return s_memoryCueMarkerSingleColor.load() ? memoryCueMarkerColor() : fillColor();
}

QColor WaveformMark::memoryCueBorderColor() const {
    if (!s_memoryCueMarkerSingleColor.load()) {
        return borderColor();
    }
    // Same rule as setBaseColor() uses for the cue's own color
    return Color::chooseContrastColor(memoryCueMarkerColor(), m_dimBrightThreshold);
}

QColor WaveformMark::memoryCueLabelColor() const {
    if (!s_memoryCueMarkerSingleColor.load()) {
        return labelColor();
    }
    return Color::chooseColorByBrightness(memoryCueMarkerColor(),
            QColor(255, 255, 255, 255),
            QColor(0, 0, 0, 255),
            m_dimBrightThreshold);
}

QColor WaveformMark::memoryCueOutlineColor() const {
    return s_memoryCueMarkerCustomOutline.load() ? memoryCueMarkerOutlineColor()
                                                 : memoryCueBorderColor();
}

// static
void WaveformMark::setMemoryCueMarkerShowNames(bool show) {
    if (s_memoryCueMarkerShowNames.exchange(show) != show) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
bool WaveformMark::memoryCueMarkerShowNames() {
    return s_memoryCueMarkerShowNames.load();
}

// static
void WaveformMark::setMemoryCueNameAlign(Qt::Alignment align) {
    const bool top = !align.testFlag(Qt::AlignBottom);
    if (s_memoryCueNameAlignTop.exchange(top) != top) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
Qt::Alignment WaveformMark::memoryCueNameAlign() {
    return s_memoryCueNameAlignTop.load() ? Qt::AlignTop : Qt::AlignBottom;
}

// static
void WaveformMark::setMemoryCueNameBackground(MemoryCueNameBackground background) {
    const int value = static_cast<int>(background);
    if (s_memoryCueNameBackground.exchange(value) != value) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
WaveformMark::MemoryCueNameBackground WaveformMark::memoryCueNameBackground() {
    switch (s_memoryCueNameBackground.load()) {
    case static_cast<int>(MemoryCueNameBackground::Off):
        return MemoryCueNameBackground::Off;
    case static_cast<int>(MemoryCueNameBackground::CustomColor):
        return MemoryCueNameBackground::CustomColor;
    default:
        return MemoryCueNameBackground::CueColor;
    }
}

// static
void WaveformMark::setMemoryCueNameBackgroundCustomColor(const QColor& color) {
    const QRgb rgba = color.isValid() ? color.rgba() : qRgba(0, 0, 0, 153);
    if (s_memoryCueNameBackgroundColor.exchange(rgba) != rgba) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
QColor WaveformMark::memoryCueNameBackgroundCustomColor() {
    return QColor::fromRgba(s_memoryCueNameBackgroundColor.load());
}

// static
void WaveformMark::setMemoryCueNameCustomTextColor(bool enabled) {
    if (s_memoryCueNameCustomTextColor.exchange(enabled) != enabled) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
bool WaveformMark::memoryCueNameCustomTextColor() {
    return s_memoryCueNameCustomTextColor.load();
}

// static
void WaveformMark::setMemoryCueNameTextColor(const QColor& color) {
    const QRgb rgb = color.isValid() ? color.rgb() : qRgb(255, 255, 255);
    if (s_memoryCueNameTextColor.exchange(rgb) != rgb) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
QColor WaveformMark::memoryCueNameTextColor() {
    return QColor::fromRgb(s_memoryCueNameTextColor.load());
}

QColor WaveformMark::memoryCueNameEffectiveTextColor() const {
    if (s_memoryCueNameCustomTextColor.load()) {
        return memoryCueNameTextColor();
    }
    switch (memoryCueNameBackground()) {
    case MemoryCueNameBackground::CueColor:
        // Readable on the cue colored box
        return memoryCueLabelColor();
    case MemoryCueNameBackground::CustomColor: {
        const QColor background = memoryCueNameBackgroundCustomColor();
        if (background.alphaF() < 0.3f) {
            // Mostly transparent: the waveform is the real background
            return memoryCueColor();
        }
        const float luminance = 0.299f * background.redF() +
                0.587f * background.greenF() + 0.114f * background.blueF();
        return luminance < 0.5f ? QColor(Qt::white) : QColor(Qt::black);
    }
    case MemoryCueNameBackground::Off:
    default:
        return memoryCueColor();
    }
}

QColor WaveformMark::memoryCueNameBackgroundColor() const {
    switch (memoryCueNameBackground()) {
    case MemoryCueNameBackground::CueColor: {
        QColor color = memoryCueColor();
        color.setAlphaF(0.6f); // subtle, the waveform stays visible underneath
        return color;
    }
    case MemoryCueNameBackground::CustomColor:
        return memoryCueNameBackgroundCustomColor();
    case MemoryCueNameBackground::Off:
    default:
        return QColor(Qt::transparent);
    }
}

// static
void WaveformMark::setMemoryCueMarkerView(MemoryCueMarkerView view) {
    const int value = static_cast<int>(view);
    if (s_memoryCueMarkerView.exchange(value) != value) {
        s_styleGeneration.fetch_add(1);
    }
}

// static
WaveformMark::MemoryCueMarkerView WaveformMark::memoryCueMarkerView() {
    switch (s_memoryCueMarkerView.load()) {
    case static_cast<int>(MemoryCueMarkerView::ScrollingWaveformsOnly):
        return MemoryCueMarkerView::ScrollingWaveformsOnly;
    case static_cast<int>(MemoryCueMarkerView::OverviewOnly):
        return MemoryCueMarkerView::OverviewOnly;
    default:
        return MemoryCueMarkerView::Both;
    }
}

bool WaveformMark::isMemoryCueMarkerInScrollingWaveforms() const {
    return isMemoryCueMarker() &&
            memoryCueMarkerView() != MemoryCueMarkerView::OverviewOnly;
}

bool WaveformMark::isMemoryCueMarkerInOverview() const {
    return isMemoryCueMarker() &&
            memoryCueMarkerView() != MemoryCueMarkerView::ScrollingWaveformsOnly;
}

bool WaveformMark::isMemoryCueMarker() const {
    if (!s_showMemoryCueMarkers.load() || m_iHotCue < kFirstMemoryCueHotCue) {
        return false;
    }
    // Saved loops keep their normal label so they stay recognizable.
    return !isLoop();
}

QImage WaveformMark::generateMemoryCueImage(float devicePixelRatio) {
    // A thin line in the cue color with a triangle at the top (pointing
    // down), at the bottom (pointing up) or both, like rekordbox memory cues.
    const int triangleSize = memoryCueMarkerSize();
    const float triangleSizeF = static_cast<float>(triangleSize);
    // An odd pixel width keeps the line centered on the cue position.
    const float width = static_cast<float>(triangleSize + (triangleSize % 2 == 0 ? 3 : 2));
    const float height = m_breadth;
    const float hcenter = width / 2.f;
    const float half = triangleSizeF / 2.f;
    const Qt::Alignment align = memoryCueMarkerAlign();

    // Optional name label to the right of the marker, at the top or bottom
    // (see memoryCueNameAlign()). Next to a triangle at the same edge it
    // starts right of the triangle, otherwise right of the line.
    QFont nameFont;
    nameFont.setPixelSize(std::clamp(triangleSize, 9, 16));
    QString name;
    QRectF nameRect;
    if (memoryCueMarkerShowNames() && !m_text.isEmpty()) {
        const QFontMetricsF metrics(nameFont);
        name = metrics.elidedText(m_text, Qt::ElideRight, kMaxMemoryCueNameWidth);
        constexpr float kPadding = 2.f;
        const float boxWidth = static_cast<float>(metrics.horizontalAdvance(name)) +
                2.f * kPadding;
        const float boxHeight = static_cast<float>(metrics.height());
        if (!name.isEmpty() && boxHeight < height) {
            const bool nameAtTop = memoryCueNameAlign().testFlag(Qt::AlignTop);
            const bool triangleAtSameEdge =
                    align.testFlag(nameAtTop ? Qt::AlignTop : Qt::AlignBottom);
            const float lineHalfWidth =
                    static_cast<float>(memoryCueMarkerLineWidth()) / 2.f;
            const float boxX = hcenter + (triangleAtSameEdge ? half : lineHalfWidth) + 2.f;
            const float boxY = nameAtTop ? 0.5f : height - boxHeight - 0.5f;
            nameRect = QRectF(boxX, boxY, boxWidth, boxHeight);
        }
    }
    // The image grows to the right for the name; the line stays at hcenter.
    const float imageWidth = nameRect.isValid()
            ? static_cast<float>(std::ceil(nameRect.right() + 1.f))
            : width;

    QImage image{QSize{static_cast<int>(imageWidth * devicePixelRatio),
                         static_cast<int>(height * devicePixelRatio)},
            QImage::Format_ARGB32_Premultiplied};
    VERIFY_OR_DEBUG_ASSERT(!image.isNull()) {
        return image;
    }
    image.setDevicePixelRatio(devicePixelRatio);
    image.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setWorldMatrixEnabled(false);

    m_linePosition = hcenter;
    m_offset = -(width - 1.f) / 2.f;

    // Line and triangles use the configured opacity, the name stays opaque.
    painter.setOpacity(memoryCueMarkerOpacity() / 100.0);

    // Line in the cue color, centered on the cue position (width 0 = no line)
    const float lineWidth = static_cast<float>(memoryCueMarkerLineWidth());
    if (lineWidth > 0.f) {
        painter.fillRect(QRectF(hcenter - lineWidth / 2.f, 0.f, lineWidth, height),
                memoryCueColor());
    }

    QPainterPath triangles;
    if (align.testFlag(Qt::AlignTop)) {
        QPainterPath top;
        top.moveTo(hcenter - half, 0.5f);
        top.lineTo(hcenter + half, 0.5f);
        top.lineTo(hcenter, triangleSizeF);
        top.closeSubpath();
        triangles.addPath(top);
    }
    if (align.testFlag(Qt::AlignBottom)) {
        QPainterPath bottom;
        bottom.moveTo(hcenter, height - triangleSizeF);
        bottom.lineTo(hcenter + half, height - 0.5f);
        bottom.lineTo(hcenter - half, height - 0.5f);
        bottom.closeSubpath();
        triangles.addPath(bottom);
    }
    painter.fillPath(triangles, memoryCueColor());
    painter.setPen(memoryCueOutlineColor());
    painter.drawPath(triangles);
    painter.setOpacity(1.0);

    QRectF hoverArea = triangles.boundingRect();
    if (nameRect.isValid()) {
        if (memoryCueNameBackground() != MemoryCueNameBackground::Off) {
            QPainterPath box;
            box.addRoundedRect(nameRect, 2.f, 2.f);
            painter.fillPath(box, memoryCueNameBackgroundColor());
        }
        painter.setFont(nameFont);
        painter.setPen(memoryCueNameEffectiveTextColor());
        painter.drawText(nameRect, Qt::AlignCenter, name);
        hoverArea = hoverArea.united(nameRect);
    }
    painter.end();

    // Hover / right-click area for the cue menu. With triangles at the top
    // and bottom this spans the whole marker height.
    m_label.setAreaRect(hoverArea);
    return image;
}

void WaveformMark::setBaseColor(QColor baseColor, int dimBrightThreshold) {
    if (m_fillColor == baseColor) {
        return;
    }

    m_fillColor = baseColor;
    m_dimBrightThreshold = dimBrightThreshold;
    m_borderColor = Color::chooseContrastColor(baseColor, dimBrightThreshold);
    m_labelColor = Color::chooseColorByBrightness(baseColor,
            QColor(255, 255, 255, 255),
            QColor(0, 0, 0, 255),
            dimBrightThreshold);

    setNeedsImageUpdate();
}

bool WaveformMark::lineHovered(QPoint point, Qt::Orientation orientation) const {
    if (orientation == Qt::Vertical) {
        // Note that for vertical orientation, breadth is set to the width.
        point = QPoint(point.y(), static_cast<int>(m_breadth) - point.x());
    }
    return m_linePosition >= point.x() - lineHoverPadding &&
            m_linePosition <= point.x() + lineHoverPadding;
}

bool WaveformMark::contains(QPoint point, Qt::Orientation orientation) const {
    if (orientation == Qt::Vertical) {
        point = QPoint(point.y(), static_cast<int>(m_breadth) - point.x());
    }
    return m_label.area().contains(point);
}

// Helper struct to calculate the geometry and fontsize needed by generateImage
// to draw the label and text
class MarkerGeometry {
  public:
    MarkerGeometry(const QString& label,
            bool useIcon,
            Qt::Alignment align,
            float breadth,
            int level) {
        // If the label is 1 character long, and this character isn't a letter or a number,
        // we can assume it's a special symbol
        m_isSymbol = !useIcon && label.length() == 1 && !label[0].isLetterOrNumber();

        // This alone would pick the OS default font, or that set by Qt5 Settings (qt5ct)
        // respectively. This would mostly not be notable since contemporary OS and distros
        // use a proven sans-serif anyway. Though, some user fonts may be lacking glyphs
        // we use for the intro/outro markers for example.
        // So, let's just use Open Sans which is used by all official skins to achieve
        // a consistent skin design.
        m_font.setFamily("Open Sans");
        // For text, use a pixel size like everywhere else in Mixxx, which can be scaled
        // well in general.
        // Point sizes would work if only explicit Qt scaling QT_SCALE_FACTORS is used,
        // though as soon as other OS-based font and app scaling mechanics join the
        // party the resulting font size is hard to predict (affects all supported OS).

        m_font.setPixelSize(13);
        m_font.setWeight(QFont::Bold);
        m_font.setItalic(false);

        const qreal margin{3.f};

        QFontMetricsF metrics{m_font};
        const qreal capHeight = metrics.capHeight();

        if (m_isSymbol) {
            // Symbols can be aligned and sized in an way that is not ideal.
            // We auto-scale the font size so the symbol fits in capHeight
            // (the height of a flat capital letter such as H) but without
            // exceeded a width of capHeight * 1.1
            const auto targetHeight = std::ceil(capHeight);
            const auto targetWidth = std::ceil(capHeight * 1.1f);

            // As a starting point we look at the tight bounding rect at a
            // large font size
            m_font.setPointSize(50.0);
            metrics = QFontMetricsF(m_font);
            m_contentRect = metrics.tightBoundingRect(label);

            // Now we calculate how much bigger this is than our target
            // size.
            const auto ratioH = targetHeight / m_contentRect.height();
            const auto ratioW = targetWidth / m_contentRect.width();
            const auto ratio = std::min(ratioH, ratioW);

            // And we scale the font size accordingly.
            m_font.setPointSizeF(50.0 * ratio);
            metrics = QFontMetricsF(m_font);
            m_contentRect = metrics.tightBoundingRect(label);
        } else if (useIcon) {
            m_contentRect = QRectF(0.f, 0.f, std::ceil(capHeight), std::ceil(capHeight));
        } else {
            m_contentRect = QRectF{0.f,
                    -capHeight,
                    metrics.boundingRect(label).width(),
                    capHeight};
        }

        const Qt::Alignment alignH = align & Qt::AlignHorizontal_Mask;
        const Qt::Alignment alignV = align & Qt::AlignVertical_Mask;
        const bool alignHCenter{alignH == Qt::AlignHCenter};

        // The image width is the label rect width + 1, so that the label rect
        // left and right positions can be at an integer + 0.5. This is so that
        // the label rect is drawn at an exact pixel positions.
        //
        // Likewise, the line position also has to fall on an integer + 0.5.
        // When center aligning, the image width has to be odd, so that the
        // center is an integer + 0.5. For the image width to be odd, to
        // label rect width has to be even.
        const qreal widthRounding{alignHCenter ? 2.f : 1.f};

        m_labelRect = QRectF{0.f,
                0.f,
                std::ceil((m_contentRect.width() + 2.f * margin) / widthRounding) *
                        widthRounding,
                std::ceil(capHeight + 2.f * margin)};

        m_imageSize = QSizeF{m_labelRect.width() + 1.f, breadth};

        m_labelRect.moveLeft(0.5f);

        const float increment = overlappingMarkerIncrement(
                static_cast<float>(m_labelRect.height()), breadth);

        if (alignV == Qt::AlignVCenter) {
            m_labelRect.moveTop((m_imageSize.height() - m_labelRect.height()) / 2.f);
        } else if (alignV == Qt::AlignBottom) {
            m_labelRect.moveBottom(m_imageSize.height() - 0.5f -
                    level * increment);
        } else {
            m_labelRect.moveTop(0.5f + level * increment);
        }
    }
    QSize getImageSize(float devicePixelRatio) const {
        return QSize{static_cast<int>(std::lround(m_imageSize.width() * devicePixelRatio)),
                static_cast<int>(std::lround(m_imageSize.height() * devicePixelRatio))};
    }

    const QFont font() const {
        return m_font;
    }

    const QRectF& contentRect() const {
        return m_contentRect;
    }

    const QRectF& labelRect() const {
        return m_labelRect;
    }

    const QSizeF& imageSize() const {
        return m_imageSize;
    }

  private:
    bool m_isSymbol; // is the label normal text or a single symbol (e.g. open circle arrow)
    QFont m_font;
    QRectF m_contentRect;
    QRectF m_labelRect;
    QSizeF m_imageSize;
};

QImage WaveformMark::performImageGeneration(float devicePixelRatio,
        const QString& pixmapPath,
        const QString& text,
        WaveformMarkLabel* labelMark,
        const QString& iconPath) {
    if (m_breadth == 0.0f) {
        return {};
    }

    // Load the pixmap from file.
    // If that succeeds loading the text and stroke is skipped.

    if (!pixmapPath.isEmpty()) {
        QString path = pixmapPath;
        // Use devicePixelRatio to properly scale the image
        QImage image = *WImageStore::getImage(path, devicePixelRatio);
        // If loading the image didn't fail, then we're done. Otherwise fall
        // through and render a label.
        if (!image.isNull()) {
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            // Set the pixel/device ratio AFTER loading the image in order to get
            // a truly scaled source image.
            // See https://doc.qt.io/qt-5/qimage.html#setDevicePixelRatio
            // Also, without this some Qt-internal issue results in an offset
            // image when calculating the center line of pixmaps in draw().
            image.setDevicePixelRatio(devicePixelRatio);
            // Calculate the offset
            const float imgw = image.width();
            const Qt::Alignment alignH = m_align & Qt::AlignHorizontal_Mask;
            switch (alignH) {
            case Qt::AlignHCenter:
                m_offset = -(imgw - 1.f) / 2.f;
                break;
            case Qt::AlignLeft:
                m_offset = -imgw + 2.f;
                break;
            case Qt::AlignRight:
            default:
                m_offset = -1.f;
                break;
            }
            return image;
        }
    }
    const bool useIcon = iconPath != "";

    // Determine drawing geometries
    const MarkerGeometry markerGeometry{text, useIcon, m_align, m_breadth, m_level};

    float linePos;
    if (labelMark) {
        labelMark->setAreaRect(markerGeometry.labelRect());

        const Qt::Alignment alignH = m_align & Qt::AlignHorizontal_Mask;
        const float imgw = static_cast<float>(markerGeometry.imageSize().width());
        switch (alignH) {
        case Qt::AlignHCenter:
            m_linePosition = imgw / 2.f;
            m_offset = -(imgw - 1.f) / 2.f;
            break;
        case Qt::AlignLeft:
            m_linePosition = imgw - 1.5f;
            m_offset = -imgw + 2.f;
            break;
        case Qt::AlignRight:
        default:
            m_linePosition = 1.5f;
            m_offset = -1.f;
            break;
        }
        linePos = m_linePosition;
    } else {
        linePos = static_cast<float>(markerGeometry.imageSize().width()) / 2.f;
    }

    const QSize size{markerGeometry.getImageSize(devicePixelRatio)};

    if (size.width() <= 0 || size.height() <= 0) {
        return QImage{};
    }

    QImage image{size, QImage::Format_ARGB32_Premultiplied};
    VERIFY_OR_DEBUG_ASSERT(!image.isNull()) {
        return image;
    }
    image.setDevicePixelRatio(devicePixelRatio);

    // Fill with transparent pixels
    image.fill(Qt::transparent);

    QPainter painter;

    painter.begin(&image);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setWorldMatrixEnabled(false);

    // Note: linePos has to be at integer + 0.5 to draw correctly
    [[maybe_unused]] const float epsilon = 1e-6f;
    DEBUG_ASSERT(std::abs(linePos - std::floor(linePos) - 0.5) < epsilon);

    // Draw the center line
    painter.setPen(fillColor());
    painter.drawLine(QLineF(linePos, 0.f, linePos, markerGeometry.imageSize().height()));

    painter.setPen(borderColor());
    painter.drawLine(QLineF(linePos - 1.f,
            0.f,
            linePos - 1.f,
            markerGeometry.imageSize().height()));
    painter.drawLine(QLineF(linePos + 1.f,
            0.f,
            linePos + 1.f,
            markerGeometry.imageSize().height()));

    if (useIcon || text.length() != 0) {
        painter.setPen(borderColor());

        // Draw the label rounded rect with border
        QPainterPath path;
        path.addRoundedRect(markerGeometry.labelRect(), 2.f, 2.f);
        painter.fillPath(path, fillColor());
        painter.drawPath(path);

        // Center m_contentRect.width() and m_contentRect.height() inside labelRectMarl
        // and apply the offset x,y so the text ends up in the centered width,height.
        QPointF pos(markerGeometry.labelRect().x() +
                        (markerGeometry.labelRect().width() -
                                markerGeometry.contentRect().width()) /
                                2.f -
                        markerGeometry.contentRect().x(),
                markerGeometry.labelRect().y() +
                        (markerGeometry.labelRect().height() -
                                markerGeometry.contentRect().height()) /
                                2.f -
                        markerGeometry.contentRect().y());

        if (useIcon) {
            QSvgRenderer svgRenderer(iconPath);
            svgRenderer.render(&painter, QRectF(pos, markerGeometry.contentRect().size()));
        } else {
            // Draw the text
            painter.setBrush(Qt::transparent);
            painter.setPen(labelColor());
            painter.setFont(markerGeometry.font());

            painter.drawText(pos, text);
        }
    }

    painter.end();

    return image;
}

std::optional<WaveformMark::WaveformMarkConstructionError> WaveformMark::validate() const {
    auto checkPath = [](const auto& path) {
        return !std::get<QString>(path).isEmpty() && !QFileInfo(std::get<QString>(path)).exists();
    };

    auto addPaths = [](auto* pPathList,
                            const QString& pathTemplate,
                            WaveformMark::WaveformMarkConstructionError
                                    pathErrorType) {
        static QRegularExpression re("%\\d+");
        auto argCount = pathTemplate.count(re);
        switch (argCount) {
        case 0:
            pPathList->emplace_back(pathTemplate, pathErrorType);
            break;
        case 1:
            pPathList->emplace_back(pathTemplate.arg(QStringLiteral("forward")), pathErrorType);
            pPathList->emplace_back(pathTemplate.arg(QStringLiteral("backward")), pathErrorType);
            break;
        default:
            return false;
        }
        return true;
    };

    std::vector<std::pair<QString, WaveformMarkConstructionError>> pathList = {
            {m_pixmapPath, WaveformMarkConstructionError::PixmapNotFound},
            {m_endPixmapPath, WaveformMarkConstructionError::EndPixmapNotFound},
            {m_iconPath, WaveformMarkConstructionError::IconNotFound},
    };
    if (!addPaths(&pathList, m_endIconPath, WaveformMarkConstructionError::EndIconNotFound)) {
        return WaveformMarkConstructionError::EndIconInvalidArgumentCount;
    }

    auto first_missing_path = std::find_if(pathList.cbegin(), pathList.cend(), checkPath);
    if (first_missing_path != pathList.cend()) {
        return std::get<WaveformMarkConstructionError>(*first_missing_path);
    }
    return {};
}

QImage WaveformMark::generateImage(float devicePixelRatio) {
    DEBUG_ASSERT(needsImageUpdate());
    m_styleGeneration = s_styleGeneration.load();

    if (m_breadth != 0.0f && isMemoryCueMarkerInScrollingWaveforms()) {
        return generateMemoryCueImage(devicePixelRatio);
    }

    QString label = m_text;

    // Determine mark text.
    if (getHotCue() >= 0) {
        if (!label.isEmpty()) {
            label.prepend(": ");
        }
        label.prepend(QString::number(getHotCue() + 1));
    }

    return performImageGeneration(devicePixelRatio, m_pixmapPath, label, &m_label, m_iconPath);
}

QImage WaveformMark::generateEndImage(float devicePixelRatio) {
    assert(needsEndImageUpdate());

    QString direction = QStringLiteral("forward");

    if (isJump() && getSampleEndPosition() > getSamplePosition()) {
        direction = QStringLiteral("backward");
    }
    return performImageGeneration(devicePixelRatio,
            m_endPixmapPath,
            "",
            nullptr,
            m_endIconPath.contains("%1") ? m_endIconPath.arg(direction) : m_endIconPath);
}
