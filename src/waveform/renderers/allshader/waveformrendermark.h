#pragma once

#include <QColor>

#include "rendergraph/geometrynode.h"
#include "rendergraph/node.h"
#include "waveform/renderers/waveformrendermarkbase.h"

class QDomNode;

namespace rendergraph {
class GeometryNode;
class Context;
}

namespace allshader {
class DigitsRenderNode;
class WaveformRenderMark;
class WaveformMarkNode;
class WaveformMarkNodeGraphics;
}

class allshader::WaveformRenderMark : public ::WaveformRenderMarkBase,
                                      public rendergraph::Node {
  public:
    explicit WaveformRenderMark(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);

    // Pure virtual from WaveformRendererAbstract, not used
    void draw(QPainter* painter, QPaintEvent* event) override final;

    bool init() override;

    void update();

    bool isSubtreeBlocked() const override;

  private:
    void updateMarkImage(WaveformMarkPointer pMark) override;

    void updatePlayPosMarkTexture(rendergraph::Context* pContext);

    void drawTriangle(QPainter* painter,
            const QBrush& fillColor,
            QPointF p1,
            QPointF p2,
            QPointF p3);

    void updateUntilMark(double playPosition, double markerPosition);
    void drawUntilMark(float x);
    float getMaxHeightForText() const;
    void updateRangeNode(rendergraph::GeometryNode* pNode,
            const QRectF& rect,
            QColor color);

    int m_beatsUntilMark;
    double m_timeUntilMark;
    double m_currentBeatPosition;
    double m_nextBeatPosition;
    std::unique_ptr<ControlProxy> m_pTimeRemainingControl;

    bool m_isSlipRenderer;

    rendergraph::Node* m_pRangeNodesParent{};
    rendergraph::Node* m_pMarkNodesParent{};

    rendergraph::GeometryNode* m_pPlayPosNode;
    float m_playPosHeight;
    float m_playPosDevicePixelRatio;

    DigitsRenderNode* m_pDigitsRenderNode{};

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};

// On the use of QPainter:
//
// The renderers in this folder are optimized to use GLSL shaders and refrain
// from using QPainter on the QOpenGLWindow, which causes degredated performance.
//
// This renderer does use QPainter (indirectly, in WaveformMark::generateImage), but
// only to draw on a QImage. This is only done once when needed and the images are
// then used as textures to be drawn with a GLSL shader.

class allshader::WaveformMarkNode : public rendergraph::GeometryNode {
  public:
    WaveformMark* m_pOwner{};

    WaveformMarkNode(WaveformMark* pOwner, rendergraph::Context* pContext, const QImage& image);
    void updateTexture(rendergraph::Context* pContext, const QImage& image);
    void update(float x, float y, float devicePixelRatio);
    float textureWidth() const {
        return m_textureWidth;
    }
    float textureHeight() const {
        return m_textureHeight;
    }

  public:
    float m_textureWidth{};
    float m_textureHeight{};
};

class allshader::WaveformMarkNodeGraphics : public ::WaveformMark::Graphics {
  public:
    WaveformMarkNodeGraphics(WaveformMark* pOwner,
            rendergraph::Context* pContext,
            const QImage& image);
    void updateTexture(rendergraph::Context* pContext, const QImage& image) {
        waveformMarkNode()->updateTexture(pContext, image);
    }
    void update(float x, float y, float devicePixelRatio) {
        waveformMarkNode()->update(x, y, devicePixelRatio);
    }
    float textureWidth() const {
        return waveformMarkNode()->textureWidth();
    }
    float textureHeight() const {
        return waveformMarkNode()->textureHeight();
    }
    void attachNode(std::unique_ptr<rendergraph::BaseNode> pNode) {
        DEBUG_ASSERT(!m_pNode);
        m_pNode = std::move(pNode);
    }
    std::unique_ptr<rendergraph::BaseNode> detachNode() {
        return std::move(m_pNode);
    }

  private:
    WaveformMarkNode* waveformMarkNode() const {
        DEBUG_ASSERT(m_pNode);
        return static_cast<WaveformMarkNode*>(m_pNode.get());
    }

    std::unique_ptr<rendergraph::BaseNode> m_pNode;
};
