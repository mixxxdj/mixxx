#pragma once

#include <QMatrix4x4>

#include "rendergraph/geometrynode.h"
#include "util/class.h"

namespace rendergraph {
class TexturedVertexUpdater;
}

namespace allshader {
class DigitsRenderNode;
}

class allshader::DigitsRenderNode : public rendergraph::GeometryNode {
  public:
    DigitsRenderNode();
    ~DigitsRenderNode();

    void updateTexture(float fontPointSize, float maxHeight, float devicePixelRatio);

    void update(const QMatrix4x4& matrix,
            float x,
            float y,
            bool multiLine,
            const QString& s1,
            const QString& s2);
    float height() const;

  private:
    float addVertices(rendergraph::TexturedVertexUpdater& vertexUpdater,
            float x,
            float y,
            const QString& s);

    int m_penWidth;
    float m_offset[13];
    float m_width[12];
    float m_fontPointSize{};
    float m_height{};
    float m_maxHeight{};
    float m_adjustedFontPointSize{};
    DISALLOW_COPY_AND_ASSIGN(DigitsRenderNode);
};
