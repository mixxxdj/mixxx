#include "waveformmark.h"

#include <QOpenGLTexture>
#include <QPainterPath>
#include <QtDebug>

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

WaveformMark::WaveformMark(const QString& group,
        QString positionControl,
        const QString& visibilityControl,
        const QString& textColor,
        const QString& markAlign,
        const QString& text,
        const QString& pixmapPath,
        const QString& iconPath,
        QColor color,
        int priority,
        int hotCue,
        const WaveformSignalColors& signalColors)
        : m_textColor(textColor),
          m_pixmapPath(pixmapPath),
          m_iconPath(iconPath),
          m_linePosition{},
          m_breadth{},
          m_level{},
          m_iPriority(priority),
          m_iHotCue(hotCue),
          m_showUntilNext{} {
    QString endPositionControl;
    QString typeControl;
    if (hotCue != Cue::kNoHotCue) {
        QString hotcueNumber = QString::number(hotCue + 1);
        positionControl = QStringLiteral("hotcue_%1_position").arg(hotcueNumber);
        endPositionControl = QStringLiteral("hotcue_%1_endposition").arg(hotcueNumber);
        typeControl = QStringLiteral("hotcue_%1_type").arg(hotcueNumber);
        m_showUntilNext = true;
    } else {
        m_showUntilNext = isShowUntilNextPositionControl(positionControl);
    }

    if (!positionControl.isEmpty()) {
        m_pPositionCO = std::make_unique<ControlProxy>(group, positionControl);
    }
    if (!endPositionControl.isEmpty()) {
        m_pEndPositionCO = std::make_unique<ControlProxy>(group, endPositionControl);
        m_pTypeCO = std::make_unique<ControlProxy>(group, typeControl);
    }

    if (!visibilityControl.isEmpty()) {
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
    if (hotCue != Cue::kNoHotCue) {
        positionControl = "hotcue_" + QString::number(hotCue + 1) + "_position";
        endPositionControl = "hotcue_" + QString::number(hotCue + 1) + "_endposition";
        typeControl = "hotcue_" + QString::number(hotCue + 1) + "_type";
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
        m_pTypeCO = std::make_unique<ControlProxy>(group, typeControl);
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

    m_iconPath = context.selectString(node, "Icon");
    if (!m_iconPath.isEmpty()) {
        m_iconPath = context.makeSkinPath(m_iconPath);
    }
}

WaveformMark::~WaveformMark() = default;

void WaveformMark::setBaseColor(QColor baseColor, int dimBrightThreshold) {
    if (m_fillColor == baseColor) {
        return;
    }

    m_fillColor = baseColor;
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

QImage WaveformMark::generateImage(float devicePixelRatio) {
    DEBUG_ASSERT(needsImageUpdate());

    if (m_breadth == 0.0f) {
        return {};
    }

    // Load the pixmap from file.
    // If that succeeds loading the text and stroke is skipped.

    if (!m_pixmapPath.isEmpty()) {
        QString path = m_pixmapPath;
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

    QString label = m_text;

    // Determine mark text.
    if (getHotCue() >= 0) {
        if (!label.isEmpty()) {
            label.prepend(": ");
        }
        label.prepend(QString::number(getHotCue() + 1));
    }

    const bool useIcon = m_iconPath != "";

    // Determine drawing geometries
    const MarkerGeometry markerGeometry{label, useIcon, m_align, m_breadth, m_level};

    m_label.setAreaRect(markerGeometry.labelRect());

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

    // Note: linePos has to be at integer + 0.5 to draw correctly
    const float linePos = m_linePosition;
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

    if (useIcon || label.length() != 0) {
        painter.setPen(borderColor());

        // Draw the label rounded rect with border
        QPainterPath path;
        path.addRoundedRect(markerGeometry.labelRect(), 2.f, 2.f);
        painter.fillPath(path, fillColor());
        painter.drawPath(path);

        // Center m_contentRect.width() and m_contentRect.height() inside m_labelRect
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
            QSvgRenderer svgRenderer(m_iconPath);
            svgRenderer.render(&painter, QRectF(pos, markerGeometry.contentRect().size()));
        } else {
            // Draw the text
            painter.setBrush(Qt::transparent);
            painter.setPen(labelColor());
            painter.setFont(markerGeometry.font());

            painter.drawText(pos, label);
        }
    }

    painter.end();

    return image;
}
