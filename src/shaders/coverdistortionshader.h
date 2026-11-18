#pragma once

#include "shaders/shader.h"

namespace mixxx {
class CoverDistortionShader;
}

/// [AI-generated] Draws a cover image in front of a blurred copy of itself.
/// The cover is warped by three independent distortions: a radial pulse for
/// the bass, a swirl for the mids and a shimmer with color fringes for the
/// highs. With all amounts at zero the cover is drawn undistorted. The
/// blurred copy fills the viewport and flows slowly, driven by a phase that
/// the caller advances with the tempo of the music. [/AI-generated]
class mixxx::CoverDistortionShader final : public mixxx::Shader {
  public:
    CoverDistortionShader() = default;
    ~CoverDistortionShader() = default;

    /// [AI-generated] Compiles the shader. Requires a current OpenGL context.
    /// [/AI-generated]
    void init();

    int positionLocation() const {
        return m_positionLocation;
    }
    int texcoordLocation() const {
        return m_texcoordLocation;
    }
    /// [AI-generated] sampler2D: the cover. [/AI-generated]
    int textureLocation() const {
        return m_textureLocation;
    }
    /// [AI-generated] sampler2D: the blurred copy of the cover.
    /// [/AI-generated]
    int blurredTextureLocation() const {
        return m_blurredTextureLocation;
    }
    /// [AI-generated] vec2: the factors that scale the viewport to the cover.
    /// [/AI-generated]
    int fitLocation() const {
        return m_fitLocation;
    }
    /// [AI-generated] vec2: the factors that scale the viewport to the part
    /// of the blurred copy that fills it. [/AI-generated]
    int backgroundFitLocation() const {
        return m_backgroundFitLocation;
    }
    /// [AI-generated] float: the width of the viewport divided by its
    /// height. [/AI-generated]
    int windowAspectLocation() const {
        return m_windowAspectLocation;
    }
    /// [AI-generated] float: the phase of the flow of the blurred copy in
    /// radians. It can wrap around at 2 pi. [/AI-generated]
    int flowPhaseLocation() const {
        return m_flowPhaseLocation;
    }
    /// [AI-generated] float: the width of the cover divided by its height.
    /// [/AI-generated]
    int coverAspectLocation() const {
        return m_coverAspectLocation;
    }
    /// [AI-generated] vec3: the bass, mid and high amounts, each 0..1.
    /// [/AI-generated]
    int amountsLocation() const {
        return m_amountsLocation;
    }
    /// [AI-generated] vec2: the slow and the fast animation phase in radians.
    /// The shader only uses integer multiples of them, so they can wrap
    /// around at 2 pi. [/AI-generated]
    int phasesLocation() const {
        return m_phasesLocation;
    }
    /// [AI-generated] float: the brightness of the cover, 0..1.
    /// [/AI-generated]
    int alphaLocation() const {
        return m_alphaLocation;
    }

  private:
    int m_positionLocation;
    int m_texcoordLocation;
    int m_textureLocation;
    int m_blurredTextureLocation;
    int m_fitLocation;
    int m_backgroundFitLocation;
    int m_windowAspectLocation;
    int m_flowPhaseLocation;
    int m_coverAspectLocation;
    int m_amountsLocation;
    int m_phasesLocation;
    int m_alphaLocation;

    DISALLOW_COPY_AND_ASSIGN(CoverDistortionShader)
};
