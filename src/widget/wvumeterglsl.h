#pragma once

#include <QOpenGLTexture>
#include <memory>

#include "shaders/textureshader.h"
#include "widget/wpixmapstore.h"
#include "widget/wvumeterbase.h"
#include "widget/wwidget.h"

class WVuMeterGLSL : public WVuMeterBase {
    Q_OBJECT
  public:
    explicit WVuMeterGLSL(QWidget* pParent = nullptr);
    ~WVuMeterGLSL() override;

  private:
    std::unique_ptr<QOpenGLTexture> m_pTextureBack;
    std::unique_ptr<QOpenGLTexture> m_pTextureVu;
    mixxx::TextureShader m_textureShader;

    void initializeGL() override;
    void cleanupGL();
    void paintGL() override;
    void drawTexture(QOpenGLTexture* texture, const QRectF& sourceRect, const QRectF& targetRect);
};
