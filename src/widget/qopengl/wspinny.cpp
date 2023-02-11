#include "widget/wspinny.h"

void WSpinny::cleanupGL() {
    makeCurrentIfNeeded();
    m_pBgTexture.reset();
    m_pMaskTexture.reset();
    m_pFgTextureScaled.reset();
    m_pGhostTextureScaled.reset();
    m_pLoadedCoverTextureScaled.reset();
    m_pQTexture.reset();
    doneCurrent();
}

void WSpinny::updateLoaderCoverGL() {
    makeCurrentIfNeeded();
    if (m_loadedCoverScaled.isNull()) {
        m_pLoadedCoverTextureScaled.reset();
    } else {
        m_pLoadedCoverTextureScaled.reset(new QOpenGLTexture(m_loadedCoverScaled.toImage()));
    }
    doneCurrent();
}

void WSpinny::renderGL() {
    makeCurrentIfNeeded();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_shaderProgram.bind();

    QMatrix4x4 matrix;
    // matrix.ortho(QRectF(0, 0, width(), height()));
    int matrixLocation = m_shaderProgram.uniformLocation("matrix");

    m_shaderProgram.setUniformValue(matrixLocation, matrix);

    if (m_pBgTexture) {
        drawTexture(m_pBgTexture.get());
    }

    if (m_bShowCover && m_pLoadedCoverTextureScaled) {
        drawTexture(m_pLoadedCoverTextureScaled.get());
    }

    if (m_pMaskTexture) {
        drawTexture(m_pMaskTexture.get());
    }

#ifdef __VINYLCONTROL__
    // Overlay the signal quality drawing if vinyl is active
    if (m_bVinylActive && m_bSignalActive) {
        // draw the last good image
        drawTexture(m_pQTexture.get());
    }
#endif

    // To rotate the foreground image around the center of the image,
    // we use the classic trick of translating the coordinate system such that
    // the origin is at the center of the image. We then rotate the coordinate system,
    // and draw the image at the corner.
    // p.translate(width() / 2, height() / 2);

    bool paintGhost = m_bGhostPlayback && m_pGhostTextureScaled;

    if (paintGhost) {
        QMatrix4x4 rotate;
        rotate.rotate(m_fAngle, 0, 0, -1);
        m_shaderProgram.setUniformValue(matrixLocation, rotate);

        drawTexture(m_pGhostTextureScaled.get());
    }

    if (m_pFgTextureScaled) {
        QMatrix4x4 rotate;
        rotate.rotate(m_fAngle, 0, 0, -1);
        m_shaderProgram.setUniformValue(matrixLocation, rotate);

        drawTexture(m_pFgTextureScaled.get());
    }

    doneCurrent();
}

void WSpinny::initializeGL() {
    if (m_pBgImage && !m_pBgImage->isNull())
        m_pBgTexture.reset(new QOpenGLTexture(*m_pBgImage));
    if (m_pMaskImage && !m_pMaskImage->isNull())
        m_pMaskTexture.reset(new QOpenGLTexture(*m_pMaskImage));
    if (!m_fgImageScaled.isNull())
        m_pFgTextureScaled.reset(new QOpenGLTexture(m_fgImageScaled));
    if (!m_ghostImageScaled.isNull())
        m_pGhostTextureScaled.reset(new QOpenGLTexture(m_ghostImageScaled));
    if (!m_loadedCoverScaled.isNull()) {
        m_pLoadedCoverTextureScaled.reset(new QOpenGLTexture(m_loadedCoverScaled.toImage()));
    }
    if (!m_qImage.isNull())
        m_pQTexture.reset(new QOpenGLTexture(m_qImage));

    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute vec4 position;
attribute vec3 texcoor;
varying vec3 vTexcoor;
void main()
{
    vTexcoor = texcoor;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform sampler2D sampler;
varying vec3 vTexcoor;
void main()
{
    gl_FragColor = texture2D(sampler, vec2(vTexcoor.x, vTexcoor.y));
}
)--");

    if (!m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderCode)) {
        return;
    }

    if (!m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderCode)) {
        return;
    }

    if (!m_shaderProgram.link()) {
        return;
    }

    if (!m_shaderProgram.bind()) {
        return;
    }
}

void WSpinny::drawTexture(QOpenGLTexture* texture) {
    const float texx1 = 0.f;
    const float texy1 = 1.f;
    const float texx2 = 1.f;
    const float texy2 = 0.f;

    const float tw = texture->width();
    const float th = texture->height();

    // fill centered
    const float posx2 = tw >= th ? 1.f : (th - tw) / th;
    const float posy2 = th >= tw ? 1.f : (tw - th) / tw;
    const float posx1 = -posx2;
    const float posy1 = -posy2;

    const float posarray[] = {posx1, posy1, posx2, posy1, posx1, posy2, posx2, posy2};
    const float texarray[] = {texx1, texy1, texx2, texy1, texx1, texy2, texx2, texy2};

    int samplerLocation = m_shaderProgram.uniformLocation("sampler");
    int positionLocation = m_shaderProgram.attributeLocation("position");
    int texcoordLocation = m_shaderProgram.attributeLocation("texcoor");

    m_shaderProgram.enableAttributeArray(positionLocation);
    m_shaderProgram.setAttributeArray(
            positionLocation, GL_FLOAT, posarray, 2);
    m_shaderProgram.enableAttributeArray(texcoordLocation);
    m_shaderProgram.setAttributeArray(
            texcoordLocation, GL_FLOAT, texarray, 2);

    m_shaderProgram.setUniformValue(samplerLocation, 0);

    texture->bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
