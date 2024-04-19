#include "waveform/renderers/allshader/digitsrenderer.h"

#include <QColor>
#include <QFontMetricsF>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QOpenGLTexture>
#include <QPainter>
#include <QPainterPath>

#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/allshader/vertexdata.h"

// Render digits using a texture (generated) with digits with blurred dark outline

namespace {

// The texture will contain 12 characters: 10 digits, colon and dot
constexpr int NUM_CHARS = 12;

// space around chars for blurred dark outline
constexpr qreal SPACE = 3;
// extra kerning when rendering chars
constexpr qreal KERNING = 1;

char indexToChar(int index) {
    const char* str = "0123456789:.";
    return str[index];
}
int charToIndex(char ch) {
    if (ch >= '0' && ch <= '9') {
        return static_cast<int>(ch - '0');
    }
    if (ch == ':') {
        return 10;
    }
    if (ch == '.') {
        return 11;
    }
    assert(false);
    return 11; // fallback to dot
}
} // namespace

allshader::DigitsRenderer::~DigitsRenderer() = default;

void allshader::DigitsRenderer::init() {
    initializeOpenGLFunctions();
    m_shader.init();
}

float allshader::DigitsRenderer::height() const {
    return static_cast<float>(m_texture.height());
}

void allshader::DigitsRenderer::generateTexture(int fontPixelSize, float devicePixelRatio) {
    QFont font;
    font.setFamily("Open Sans");
    font.setPixelSize(fontPixelSize);

    QFontMetricsF metrics{font};

    qreal totalTextWidth = 0;
    qreal maxTextHeight = 0;
    for (int i = 0; i < NUM_CHARS; i++) {
        assert(charToIndex(indexToChar(i)) == i);
        const QString text(indexToChar(i));
        const auto rect = metrics.tightBoundingRect(text);
        maxTextHeight = std::max(maxTextHeight, rect.height());
        totalTextWidth += metrics.horizontalAdvance(text) + SPACE + SPACE;
    }

    totalTextWidth = std::ceil(totalTextWidth);

    QImage image(static_cast<int>(totalTextWidth * devicePixelRatio),
            static_cast<int>(std::ceil(maxTextHeight + SPACE + SPACE) * devicePixelRatio),
            QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(devicePixelRatio);
    image.fill(Qt::transparent);

    // Draw digits with dark outline
    QPainter painter;
    QPen pen(QColor(0, 0, 0, 192));
    pen.setWidth(3);

    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setWorldMatrixEnabled(false);
    painter.setBrush(QColor(0, 0, 0, 224));
    painter.setPen(pen);
    painter.setFont(font);
    qreal x = 0;
    for (int i = 0; i < NUM_CHARS; i++) {
        const QString text(indexToChar(i));
        QPainterPath path;
        path.addText(QPointF(x + SPACE, maxTextHeight + SPACE), font, text);
        painter.drawPath(path);
        x += metrics.horizontalAdvance(text) + SPACE + SPACE;
    }
    painter.end();

    // Apply Gaussian blur to dark outline

    QGraphicsBlurEffect* blur = new QGraphicsBlurEffect; // ownership passed to item
    blur->setBlurRadius(3);

    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(image));
    item.setGraphicsEffect(blur);
    scene.addItem(&item);

    image.fill(Qt::transparent);
    painter.begin(&image);
    scene.render(&painter, QRectF(), QRectF(0, 0, image.width(), image.height()));

    // Draw digits foreground
    painter.setPen(Qt::white);
    painter.setFont(font);

    x = 0;
    for (int i = 0; i < NUM_CHARS; i++) {
        const QString text(indexToChar(i));
        painter.drawText(QPointF(x + SPACE, maxTextHeight + SPACE), text);
        // position and width of character at index i in the texture
        m_offset[i] = static_cast<float>(x / totalTextWidth);
        m_width[i] = static_cast<float>(metrics.horizontalAdvance(text) + SPACE + SPACE);
        x += metrics.horizontalAdvance(text) + SPACE + SPACE;
    }
    m_offset[NUM_CHARS] = 1.f;

    painter.end();

    m_texture.setData(image);
}

float allshader::DigitsRenderer::draw(const QMatrix4x4& matrix,
        float x,
        float y,
        const QString& s,
        float devicePixelRatio) {
    const int n = s.length();
    const float x0 = x;
    const float dx = static_cast<float>(-(SPACE + SPACE) + KERNING);

    VertexData posVertices;
    VertexData texVertices;

    posVertices.reserve(n * 6); // two triangles per character
    texVertices.reserve(n * 6);

    for (const auto c : s.toUtf8()) {
        // add vertices for the rectangles in the texture corresponding with each char in QString s
        int index = charToIndex(c);

        texVertices.addRectangle(m_offset[index], 0.f, m_offset[index + 1], 1.f);
        posVertices.addRectangle(x,
                y,
                x + m_width[index],
                y + height() / devicePixelRatio);
        x += m_width[index] + dx;
    }

    m_shader.bind();

    const int matrixLocation = m_shader.uniformLocation("matrix");
    const int textureLocation = m_shader.uniformLocation("texture");
    const int positionLocation = m_shader.attributeLocation("position");
    const int texcoordLocation = m_shader.attributeLocation("texcoord");

    m_shader.setUniformValue(matrixLocation, matrix);

    m_shader.enableAttributeArray(positionLocation);
    m_shader.setAttributeArray(
            positionLocation, GL_FLOAT, posVertices.constData(), 2);
    m_shader.enableAttributeArray(texcoordLocation);
    m_shader.setAttributeArray(
            texcoordLocation, GL_FLOAT, texVertices.constData(), 2);

    m_shader.setUniformValue(textureLocation, 0);

    m_texture.bind();

    glDrawArrays(GL_TRIANGLES, 0, posVertices.size());

    m_texture.release();

    m_shader.disableAttributeArray(positionLocation);
    m_shader.disableAttributeArray(texcoordLocation);
    m_shader.release();

    return x - x0;
}
