#pragma once

#include <QOpenGLFunctions>

#include "shaders/textureshader.h"
#include "shaders/vinylqualityshader.h"
#include "widget/wspinnybase.h"

class QOpenGLTexture;

class WSpinnyGLSL : public WSpinnyBase, private QOpenGLFunctions {
    Q_OBJECT
  public:
    WSpinnyGLSL(QWidget* parent,
            const QString& group,
            UserSettingsPointer pConfig,
            VinylControlManager* pVCMan,
            BaseTrackPlayer* pPlayer);
    ~WSpinnyGLSL() override;

  private:
    void draw() override;
    void coverChanged() override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void drawTexture(QOpenGLTexture* texture);
    void cleanupGL();
    void updateTextures();

    void setupVinylSignalQuality() override;
    void updateVinylSignalQualityImage(
            const QColor& qual_color, const unsigned char* data) override;
    void drawVinylQuality();

    mixxx::TextureShader m_textureShader;
    mixxx::VinylQualityShader m_vinylQualityShader;
    std::unique_ptr<QOpenGLTexture> m_pBgTexture;
    std::unique_ptr<QOpenGLTexture> m_pMaskTexture;
    std::unique_ptr<QOpenGLTexture> m_pFgTextureScaled;
    std::unique_ptr<QOpenGLTexture> m_pGhostTextureScaled;
    std::unique_ptr<QOpenGLTexture> m_pLoadedCoverTextureScaled;
    std::unique_ptr<QOpenGLTexture> m_pQTexture;
    QColor m_vinylQualityColor;
};
