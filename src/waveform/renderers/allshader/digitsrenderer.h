#pragma once

#include "rendergraph/context.h"
#include "rendergraph/geometrynode.h"
#include "util/class.h"

namespace rendergraph {
class TexturedVertexUpdater;
} // namespace rendergraph

namespace allshader {
class DigitsRenderNode;
} // namespace allshader

class allshader::DigitsRenderNode : public rendergraph::GeometryNode {
  public:
    DigitsRenderNode();
    ~DigitsRenderNode();

    void updateTexture(rendergraph::Context* pContext,
            float fontPointSize,
            float maxHeight,
            float devicePixelRatio);

    void update(
            float x,
            float y,
            bool multiLine,
            const QString& s1,
            const QString& s2);

    void clear();

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
