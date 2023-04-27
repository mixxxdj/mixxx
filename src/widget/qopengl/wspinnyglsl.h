#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "waveform/renderers/qopengl/shaders/textureshader.h"
#include "widget/wspinnybase.h"

class WSpinnyGLSL : public WSpinnyBase {
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
    void renderGL() override;
    void drawTexture(QOpenGLTexture* texture);
    void cleanupGL();
    void updateLoaderCoverGL();

    qopengl::TextureShader m_textureShader;
    std::unique_ptr<QOpenGLTexture> m_pBgTexture;
    std::unique_ptr<QOpenGLTexture> m_pMaskTexture;
    std::unique_ptr<QOpenGLTexture> m_pFgTextureScaled;
    std::unique_ptr<QOpenGLTexture> m_pGhostTextureScaled;
    std::unique_ptr<QOpenGLTexture> m_pLoadedCoverTextureScaled;
    std::unique_ptr<QOpenGLTexture> m_pQTexture;
};
