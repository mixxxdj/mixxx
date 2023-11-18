#include "waveformmark.h"

#include <QOpenGLTexture>
#include <QPainterPath>
#include <QtDebug>

#include "skin/legacy/skincontext.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "widget/wimagestore.h"
#include "widget/wskincolor.h"

namespace {
Qt::Alignment decodeAlignmentFlags(const QString& alignString, Qt::Alignment defaultFlags) {
    QStringList stringFlags = alignString.toLower()
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
} // anonymous namespace

WaveformMark::WaveformMark(const QString& group,
        const QDomNode& node,
        const SkinContext& context,
        const WaveformSignalColors& signalColors,
        int hotCue)
        : m_linePosition{}, m_breadth{}, m_iHotCue{hotCue} {
    QString positionControl;
    QString endPositionControl;
    if (hotCue != Cue::kNoHotCue) {
        positionControl = "hotcue_" + QString::number(hotCue + 1) + "_position";
        endPositionControl = "hotcue_" + QString::number(hotCue + 1) + "_endposition";
    } else {
        positionControl = context.selectString(node, "Control");
    }

    if (!positionControl.isEmpty()) {
        m_pPositionCO = std::make_unique<ControlProxy>(group, positionControl);
    }
    if (!endPositionControl.isEmpty()) {
        m_pEndPositionCO = std::make_unique<ControlProxy>(group, endPositionControl);
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
    if (m_pGraphics) {
        m_pGraphics->m_obsolete = true;
    }
    m_fillColor = baseColor;
    m_borderColor = Color::chooseContrastColor(baseColor, dimBrightThreshold);
    m_labelColor = Color::chooseColorByBrightness(baseColor,
            QColor(255, 255, 255, 255),
            QColor(0, 0, 0, 255),
            dimBrightThreshold);
};

bool WaveformMark::contains(QPoint point, Qt::Orientation orientation) const {
    // Without some padding, the user would only have a single pixel width that
    // would count as hovering over the WaveformMark.
    float lineHoverPadding = 5.0;
    if (orientation == Qt::Vertical) {
        point = QPoint(point.y(), m_breadth - point.x());
    }
    bool lineHovered = m_linePosition >= point.x() - lineHoverPadding &&
            m_linePosition <= point.x() + lineHoverPadding;

    return m_label.area().contains(point) || lineHovered;
}

// Helper struct to calculate the geometry and fontsize needed by generateImage
// to draw the label and text
struct MarkerGeometry {
    bool m_isSymbol; // it the label normal text or a single symbol (e.g. open circle arrow)
    QFont m_font;
    QRectF m_contentRect;
    QRectF m_labelRect;
    QSizeF m_imageSize;

    MarkerGeometry(const QString& label, bool useIcon, Qt::Alignment align, float breadth) {
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
        const qreal widthRounding{alignHCenter ? 2.f : 1.f};

        m_labelRect = QRectF{0.f,
                0.f,
                std::ceil((m_contentRect.width() + 2.f * margin) / widthRounding) *
                        widthRounding,
                std::ceil(capHeight + 2.f * margin)};

        m_imageSize = QSizeF{alignHCenter ? m_labelRect.width() + 1.f
                                          : 2.f * m_labelRect.width() + 1.f,
                breadth};

        if (alignH == Qt::AlignHCenter) {
            m_labelRect.moveLeft((m_imageSize.width() - m_labelRect.width()) / 2.f);
        } else if (alignH == Qt::AlignRight) {
            m_labelRect.moveRight(m_imageSize.width() - 0.5f);
        } else {
            m_labelRect.moveLeft(0.5f);
        }

        if (alignV == Qt::AlignVCenter) {
            m_labelRect.moveTop((m_imageSize.height() - m_labelRect.height()) / 2.f);
        } else if (alignV == Qt::AlignBottom) {
            m_labelRect.moveBottom(m_imageSize.height() - 0.5f);
        } else {
            m_labelRect.moveTop(0.5f);
        }
    }
    QSize getImageSize(float devicePixelRatio) const {
        return QSize{static_cast<int>(m_imageSize.width() * devicePixelRatio),
                static_cast<int>(m_imageSize.height() * devicePixelRatio)};
    }
};

QImage WaveformMark::generateImage(float breadth, float devicePixelRatio) {
    // Load the pixmap from file.
    // If that succeeds loading the text and stroke is skipped.

    m_breadth = static_cast<int>(breadth);

    if (!m_pixmapPath.isEmpty()) {
        QString path = m_pixmapPath;
        // Use devicePixelRatio to properly scale the image
        QImage image = *WImageStore::getImage(path, devicePixelRatio);
        //  If loading the image didn't fail, then we're done. Otherwise fall
        //  through and render a label.
        if (!image.isNull()) {
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            //  Set the pixel/device ratio AFTER loading the image in order to get
            //  a truly scaled source image.
            //  See https://doc.qt.io/qt-5/qimage.html#setDevicePixelRatio
            //  Also, without this some Qt-internal issue results in an offset
            //  image when calculating the center line of pixmaps in draw().
            image.setDevicePixelRatio(devicePixelRatio);
            return image;
        }
    }

    QString label = m_text;

    // Determine mark text.
    if (getHotCue() >= 0) {
        constexpr int kMaxCueLabelLength = 23;
        if (!label.isEmpty()) {
            label.prepend(": ");
        }
        label.prepend(QString::number(getHotCue() + 1));
        if (label.size() > kMaxCueLabelLength) {
            label = label.left(kMaxCueLabelLength - 3) + "...";
        }
    }

    const bool useIcon = m_iconPath != "";

    // Determine drawing geometries
    const MarkerGeometry markerGeometry(label, useIcon, m_align, breadth);

    m_label.setAreaRect(markerGeometry.m_labelRect);

    // Create the image
    QImage image{markerGeometry.getImageSize(devicePixelRatio),
            QImage::Format_ARGB32_Premultiplied};
    image.setDevicePixelRatio(devicePixelRatio);

    // Fill with transparent pixels
    image.fill(Qt::transparent);

    QPainter painter;

    painter.begin(&image);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setWorldMatrixEnabled(false);

    // Draw marker lines
    const auto hcenter = markerGeometry.m_imageSize.width() / 2.f;
    m_linePosition = static_cast<float>(hcenter);

    // Draw the center line
    painter.setPen(fillColor());
    painter.drawLine(QLineF(hcenter, 0.f, hcenter, markerGeometry.m_imageSize.height()));

    painter.setPen(borderColor());
    painter.drawLine(QLineF(hcenter - 1.f,
            0.f,
            hcenter - 1.f,
            markerGeometry.m_imageSize.height()));
    painter.drawLine(QLineF(hcenter + 1.f,
            0.f,
            hcenter + 1.f,
            markerGeometry.m_imageSize.height()));

    if (useIcon || label.length() != 0) {
        painter.setPen(borderColor());

        // Draw the label rounded rect with border
        QPainterPath path;
        path.addRoundedRect(markerGeometry.m_labelRect, 2.f, 2.f);
        painter.fillPath(path, fillColor());
        painter.drawPath(path);

        // Center m_contentRect.width() and m_contentRect.height() inside m_labelRect
        // and apply the offset x,y so the text ends up in the centered width,height.
        QPointF pos(markerGeometry.m_labelRect.x() +
                        (markerGeometry.m_labelRect.width() -
                                markerGeometry.m_contentRect.width()) /
                                2.f -
                        markerGeometry.m_contentRect.x(),
                markerGeometry.m_labelRect.y() +
                        (markerGeometry.m_labelRect.height() -
                                markerGeometry.m_contentRect.height()) /
                                2.f -
                        markerGeometry.m_contentRect.y());

        if (useIcon) {
            QSvgRenderer svgRenderer(m_iconPath);
            svgRenderer.render(&painter, QRectF(pos, markerGeometry.m_contentRect.size()));
        } else {
            // Draw the text
            painter.setBrush(Qt::transparent);
            painter.setPen(labelColor());
            painter.setFont(markerGeometry.m_font);

            painter.drawText(pos, label);
        }
    }

    painter.end();

    return image;
}
