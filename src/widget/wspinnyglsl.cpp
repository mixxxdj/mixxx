#include "widget/wspinnyglsl.h"

#include "moc_wspinnyglsl.cpp"
#include "util/assert.h"
#include "util/texture.h"

WSpinnyGLSL::WSpinnyGLSL(
        QWidget* parent,
        const QString& group,
        UserSettingsPointer pConfig,
        VinylControlManager* pVCMan,
        BaseTrackPlayer* pPlayer)
        : WSpinnyBase(parent, group, pConfig, pVCMan, pPlayer) {
}

WSpinnyGLSL::~WSpinnyGLSL() {
    cleanupGL();
}

void WSpinnyGLSL::cleanupGL() {
    makeCurrentIfNeeded();
    m_pBgTexture.reset();
    m_pMaskTexture.reset();
    m_pFgTextureScaled.reset();
    m_pGhostTextureScaled.reset();
    m_pLoadedCoverTextureScaled.reset();
    m_pQTexture.reset();
    doneCurrent();
}

void WSpinnyGLSL::coverChanged() {
    if (isContextValid()) {
        makeCurrentIfNeeded();
        m_pLoadedCoverTextureScaled = createTexture(m_loadedCoverScaled);
        doneCurrent();
    }
    // otherwise this will happen in initializeGL
}

void WSpinnyGLSL::draw() {
    if (shouldRender()) {
        makeCurrentIfNeeded();
        paintGL();
        doneCurrent();
    }
}

void WSpinnyGLSL::resizeGL(int w, int h) {
    Q_UNUSED(w);
    Q_UNUSED(h);
    // The images were resized in WSpinnyBase::resizeEvent.
    updateTextures();
}

void WSpinnyGLSL::updateTextures() {
    m_pBgTexture = createTexture(m_pBgImage);
    m_pMaskTexture = createTexture(m_pMaskImage);
    m_pFgTextureScaled = createTexture(m_fgImageScaled);
    m_pGhostTextureScaled = createTexture(m_ghostImageScaled);
    m_pLoadedCoverTextureScaled = createTexture(m_loadedCoverScaled);
}

void WSpinnyGLSL::setupVinylSignalQuality() {
}

void WSpinnyGLSL::updateVinylSignalQualityImage(
        const QColor& qual_color, const unsigned char* data) {
    m_vinylQualityColor = qual_color;
    if (m_pQTexture) {
        makeCurrentIfNeeded();
        m_pQTexture->bind();
        // Using a texture of one byte per pixel so we can store the vinyl
        // signal quality data directly. The VinylQualityShader will draw this
        // colorized with alpha transparency.
        glTexSubImage2D(GL_TEXTURE_2D,
                0,
                0,
                0,
                m_iVinylScopeSize,
                m_iVinylScopeSize,
                GL_RED,
                GL_UNSIGNED_BYTE,
                data);
        m_pQTexture->release();
        doneCurrent();
    }
}

void WSpinnyGLSL::paintGL() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_textureShader.bind();

    int matrixLocation = m_textureShader.matrixLocation();
    int textureLocation = m_textureShader.textureLocation();
    int positionLocation = m_textureShader.positionLocation();
    int texcoordLocation = m_textureShader.texcoordLocation();

    QMatrix4x4 matrix;
    m_textureShader.setUniformValue(matrixLocation, matrix);

    m_textureShader.enableAttributeArray(positionLocation);
    m_textureShader.enableAttributeArray(texcoordLocation);

    m_textureShader.setUniformValue(textureLocation, 0);

    if (m_pBgTexture) {
        drawTexture(m_pBgTexture.get());
    }

    if (m_bShowCover && m_pLoadedCoverTextureScaled) {
        drawTexture(m_pLoadedCoverTextureScaled.get());
    }

    if (m_pMaskTexture) {
        drawTexture(m_pMaskTexture.get());
    }

    // Overlay the signal quality drawing if vinyl is active
    if (shouldDrawVinylQuality()) {
        m_textureShader.release();
        drawVinylQuality();
        m_textureShader.bind();
    }

    // To rotate the foreground image around the center of the image,
    // we use the classic trick of translating the coordinate system such that
    // the origin is at the center of the image. We then rotate the coordinate system,
    // and draw the image at the corner.
    // p.translate(width() / 2, height() / 2);

    bool paintGhost = m_bGhostPlayback && m_pGhostTextureScaled;

    if (paintGhost) {
        QMatrix4x4 rotate;
        rotate.rotate(m_fGhostAngle, 0, 0, -1);
        m_textureShader.setUniformValue(matrixLocation, rotate);

        drawTexture(m_pGhostTextureScaled.get());
    }

    if (m_pFgTextureScaled) {
        QMatrix4x4 rotate;
        rotate.rotate(m_fAngle, 0, 0, -1);
        m_textureShader.setUniformValue(matrixLocation, rotate);

        drawTexture(m_pFgTextureScaled.get());
    }

    m_textureShader.release();
}

void WSpinnyGLSL::initializeGL() {
    initializeOpenGLFunctions();

    updateTextures();

    m_pQTexture.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
    m_pQTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    m_pQTexture->setSize(m_iVinylScopeSize, m_iVinylScopeSize);
    m_pQTexture->setFormat(QOpenGLTexture::R8_UNorm);
    m_pQTexture->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt8);

    m_textureShader.init();
    m_vinylQualityShader.init();
}

void WSpinnyGLSL::drawTexture(QOpenGLTexture* texture) {
    const float texx1 = 0.f;
    const float texy1 = 1.0;
    const float texx2 = 1.f;
    const float texy2 = 0.f;

    const float tw = texture->width();
    const float th = texture->height();

    // fill centered
    const float posx2 = tw >= th ? 1.f : tw / th;
    const float posy2 = th >= tw ? 1.f : th / tw;
    const float posx1 = -posx2;
    const float posy1 = -posy2;

    const std::array<float, 8> posarray = {posx1, posy1, posx2, posy1, posx1, posy2, posx2, posy2};
    const std::array<float, 8> texarray = {texx1, texy1, texx2, texy1, texx1, texy2, texx2, texy2};

    int positionLocation = m_textureShader.positionLocation();
    int texcoordLocation = m_textureShader.texcoordLocation();

    m_textureShader.setAttributeArray(
            positionLocation, GL_FLOAT, posarray.data(), 2);
    m_textureShader.setAttributeArray(
            texcoordLocation, GL_FLOAT, texarray.data(), 2);

    texture->bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    texture->release();
}

void WSpinnyGLSL::drawVinylQuality() {
    const float texx1 = 0.f;
    const float texy1 = 1.f;
    const float texx2 = 1.f;
    const float texy2 = 0.f;

    const float posx2 = 1.f;
    const float posy2 = 1.f;
    const float posx1 = -1.f;
    const float posy1 = -1.f;

    const std::array<float, 8> posarray = {posx1, posy1, posx2, posy1, posx1, posy2, posx2, posy2};
    const std::array<float, 8> texarray = {texx1, texy1, texx2, texy1, texx1, texy2, texx2, texy2};

    m_vinylQualityShader.bind();
    int matrixLocation = m_vinylQualityShader.matrixLocation();
    int colorLocation = m_vinylQualityShader.colorLocation();
    int textureLocation = m_vinylQualityShader.textureLocation();
    int positionLocation = m_vinylQualityShader.positionLocation();
    int texcoordLocation = m_vinylQualityShader.texcoordLocation();

    QMatrix4x4 matrix;
    m_vinylQualityShader.setUniformValue(matrixLocation, matrix);
    m_vinylQualityShader.setUniformValue(colorLocation, m_vinylQualityColor);

    m_vinylQualityShader.enableAttributeArray(positionLocation);
    m_vinylQualityShader.enableAttributeArray(texcoordLocation);

    m_vinylQualityShader.setUniformValue(textureLocation, 0);

    m_vinylQualityShader.setAttributeArray(
            positionLocation, GL_FLOAT, posarray.data(), 2);
    m_vinylQualityShader.setAttributeArray(
            texcoordLocation, GL_FLOAT, texarray.data(), 2);

    m_pQTexture->bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_pQTexture->release();

    m_vinylQualityShader.release();
}
