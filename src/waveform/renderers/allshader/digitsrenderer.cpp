#include "waveform/renderers/allshader/digitsrenderer.h"

#include <QColor>
#include <QFontMetricsF>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPath>
#include <cmath>

#include "rendergraph/context.h"
#include "rendergraph/geometry.h"
#include "rendergraph/material/texturematerial.h"
#include "rendergraph/vertexupdaters/texturedvertexupdater.h"
#include "util/assert.h"
#include "util/roundtopixel.h"

// Render digits using a texture (generated) with digits with blurred dark outline

using namespace rendergraph;

namespace {

// The texture will contain 12 characters: 10 digits, colon and dot
constexpr int NUM_CHARS = 12;

// space around chars for blurred dark outline
constexpr int OUTLINE_SIZE = 4;
// alpha of the blurred dark outline
constexpr int OUTLINE_ALPHA = 224;

constexpr char indexToChar(int index) {
    constexpr char str[] = "0123456789:.";
    return str[index];
}
constexpr int charToIndex(QChar ch) {
    int value = ch.toLatin1() - '0';
    if (value >= 0 && value <= 9) {
        return value;
    }
    if (ch == ':') {
        return 10;
    }
    if (ch == '.') {
        return 11;
    }
    DEBUG_ASSERT(false);
    return 11; // fallback to dot
}
constexpr bool checkCharToIndex() {
    for (int i = 0; i < NUM_CHARS; i++) {
        if (charToIndex(indexToChar(i)) != i) {
            return false;
        }
    }
    return true;
}
static_assert(checkCharToIndex());

} // namespace

allshader::DigitsRenderNode::DigitsRenderNode() {
    setGeometry(std::make_unique<Geometry>(TextureMaterial::attributes(), 0));
    setMaterial(std::make_unique<TextureMaterial>());
    geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
}

allshader::DigitsRenderNode::~DigitsRenderNode() = default;

float allshader::DigitsRenderNode::height() const {
    return m_height;
}

void allshader::DigitsRenderNode::updateTexture(rendergraph::Context* pContext,
        float fontPointSize,
        float maxHeight,
        float devicePixelRatio) {
    if (fontPointSize == m_fontPointSize && maxHeight == m_maxHeight) {
        return;
    }
    if (maxHeight != m_maxHeight) {
        m_maxHeight = maxHeight;
        m_adjustedFontPointSize = 0.f;
    }
    if (m_fontPointSize != fontPointSize) {
        m_fontPointSize = fontPointSize;
        if (m_adjustedFontPointSize != 0.f && fontPointSize > m_adjustedFontPointSize) {
            fontPointSize = m_adjustedFontPointSize;
        } else {
            m_adjustedFontPointSize = 0.f;
        }
    }

    float space;

    QFont font;
    QFontMetricsF metrics{font};
    font.setFamily("Open Sans");
    float maxTextHeight;
    bool retry = false;
    do {
        // At small sizes, we need to limit the pen width, to avoid drawing artifacts.
        // (The factor 0.25 was found with trial and error)
        const int maxPenWidth = 1 + std::lround(fontPointSize * 0.25f);
        // The pen width is twice the outline size
        m_penWidth = std::min(maxPenWidth, OUTLINE_SIZE * 2);

        space = static_cast<float>(m_penWidth) / 2;
        font.setPointSizeF(fontPointSize);

        const float maxHeightWithoutSpace = std::floor(maxHeight) - space * 2 - 1;

        metrics = QFontMetricsF{font};

        maxTextHeight = 0;

        for (int i = 0; i < NUM_CHARS; i++) {
            const QString text(indexToChar(i));
            const auto rect = metrics.tightBoundingRect(text);
            maxTextHeight = std::max(maxTextHeight, static_cast<float>(rect.height()));
        }
        if (m_adjustedFontPointSize == 0.f && !retry && maxTextHeight > maxHeightWithoutSpace) {
            // We need to adjust the font size to fit in the maxHeight.
            // Only do this once.
            fontPointSize *= static_cast<float>(maxHeightWithoutSpace / maxTextHeight);
            // Avoid becoming unreadable
            fontPointSize = std::max(10.f, fontPointSize);
            m_adjustedFontPointSize = fontPointSize;
            retry = true;
        } else {
            retry = false;
        }
    } while (retry);

    m_height = static_cast<float>(std::ceil(maxTextHeight)) + space * 2.f + 1.f;

    const float y = maxTextHeight + space - 0.5f;

    auto roundToPixel = createFunctionRoundToPixel(devicePixelRatio);

    float totalTextWidth{};
    std::array<float, NUM_CHARS> xs;
    // determine x position and with of each of the chars in the texture image.
    for (int i = 0; i < NUM_CHARS; i++) {
        xs[i] = totalTextWidth;
        float w = roundToPixel(static_cast<float>(
                          metrics.horizontalAdvance(indexToChar(i)))) +
                space + space + 1.f;
        totalTextWidth += w;
        m_width[i] = static_cast<float>(w);
    }
    for (int i = 0; i < NUM_CHARS; i++) {
        // position of character at index i in the texture, normalized
        m_offset[i] = static_cast<float>(xs[i] / totalTextWidth);
    }
    m_offset[NUM_CHARS] = 1.f;

    QImage image(std::lround(totalTextWidth * devicePixelRatio),
            std::lround(m_height * devicePixelRatio),
            QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(devicePixelRatio);
    image.fill(Qt::transparent);

    {
        // Draw digits with dark outline
        QPainter painter(&image);

        QPen pen(QColor(0, 0, 0, OUTLINE_ALPHA));
        pen.setWidth(m_penWidth);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(0, 0, 0, OUTLINE_ALPHA));
        painter.setPen(pen);
        painter.setFont(font);
        QPainterPath path;
        for (int i = 0; i < NUM_CHARS; i++) {
            const QString text(indexToChar(i));
            path.addText(QPointF(xs[i] + space + 0.5, y), font, text);
        }
        painter.drawPath(path);
    }

    {
        // Apply Gaussian blur to dark outline
        auto blur = std::make_unique<QGraphicsBlurEffect>();
        blur->setBlurRadius(static_cast<float>(m_penWidth) / 3);

        QGraphicsScene scene;
        QGraphicsPixmapItem item;
        item.setPixmap(QPixmap::fromImage(image));
        item.setGraphicsEffect(blur.release());
        image.fill(Qt::transparent);
        QPainter painter(&image);
        scene.addItem(&item);
        scene.render(&painter, QRectF(), QRectF(0, 0, image.width(), image.height()));
    }

    {
        // Draw digits foreground
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.setBrush(Qt::white);

        QPainterPath path;
        for (int i = 0; i < NUM_CHARS; i++) {
            const QString text(indexToChar(i));
            path.addText(QPointF(xs[i] + space + 0.5, y), font, text);
        }
        painter.drawPath(path);
    }

    dynamic_cast<TextureMaterial&>(material())
            .setTexture(std::make_unique<Texture>(pContext, image));
}

void allshader::DigitsRenderNode::update(
        float x,
        float y,
        bool multiLine,
        const QString& s1,
        const QString& s2) {
    const int numVerticesPerRectangle = 6;
    const int reserved = (s1.length() + s2.length()) * numVerticesPerRectangle;
    geometry().allocate(reserved);
    TexturedVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::TexturedPoint2D>()};

    const float ch = height();
    if (!s1.isEmpty()) {
        const auto w = addVertices(vertexUpdater,
                x,
                y,
                s1);
        if (multiLine) {
            y += ch;
        } else {
            x += w + ch * 0.75f;
        }
    }
    if (!s2.isEmpty()) {
        addVertices(vertexUpdater,
                x,
                y,
                s2);
    }

    DEBUG_ASSERT(reserved == vertexUpdater.index());
}

void allshader::DigitsRenderNode::clear() {
    geometry().allocate(0);
}

float allshader::DigitsRenderNode::addVertices(TexturedVertexUpdater& vertexUpdater,
        float x,
        float y,
        const QString& s) {
    const float x0 = x;
    const float space = static_cast<float>(m_penWidth) / 2;

    for (QChar c : s) {
        if (x != x0) {
            x -= space;
        }
        int index = charToIndex(c);

        vertexUpdater.addRectangle({x, y},
                {x + m_width[index], y + height()},
                {m_offset[index], 0.f},
                {m_offset[index + 1], 1.f});
        x += m_width[index];
    }

    return x - x0;
}
