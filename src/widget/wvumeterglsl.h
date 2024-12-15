#pragma once

#include <QOpenGLFunctions>
#include <memory>

#include "shaders/textureshader.h"
#include "util/opengltexture2d.h"
#include "widget/wvumeterbase.h"

class QOpenGLTexture;

class WVuMeterGLSL : public WVuMeterBase, private QOpenGLFunctions {
    Q_OBJECT
  public:
    explicit WVuMeterGLSL(QWidget* pParent = nullptr);
    ~WVuMeterGLSL() override;

  private:
    OpenGLTexture2D m_textureBack;
    OpenGLTexture2D m_textureVu;
    mixxx::TextureShader m_textureShader;

    void draw() override;
    void initializeGL() override;
    void cleanupGL();
    void paintGL() override;
    void drawTexture(QOpenGLTexture* texture, const QRectF& sourceRect, const QRectF& targetRect);
};
