#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <memory>

#include "waveform/renderers/qopengl/shaders/textureshader.h"
#include "widget/wpixmapstore.h"
#include "widget/wvumeterbase.h"
#include "widget/wwidget.h"

class WVuMeterGLSL : public WVuMeterBase {
    Q_OBJECT
  public:
    explicit WVuMeterGLSL(QWidget* parent = nullptr);
    ~WVuMeterGLSL() override;

  private:
    std::unique_ptr<QOpenGLTexture> m_pTextureBack;
    std::unique_ptr<QOpenGLTexture> m_pTextureVu;
    qopengl::TextureShader m_textureShader;

    void draw() override;
    void initializeGL() override;
    void cleanupGL();
    void renderGL() override;
    void drawTexture(QOpenGLTexture* texture, const QRectF& sourceRect, const QRectF& targetRect);
};
