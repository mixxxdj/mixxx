#pragma once

#include "shaders/shader.h"

namespace mixxx {
class CoverDistortionShader;
}

/// Draws a cover image in front of a blurred copy of itself.
/// The cover is warped by three independent distortions: a radial pulse for
/// the bass, a swirl for the mids and a shimmer with color fringes for the
/// highs. With all amounts at zero the cover is drawn undistorted. The
/// blurred copy fills the viewport and flows slowly, driven by a phase that
/// the caller advances with the tempo of the music. The colors of both
/// textures are divided by their alpha channel, as CoverCombineShader
/// requires for the pictures it produces.
class mixxx::CoverDistortionShader final : public mixxx::Shader {
  public:
    CoverDistortionShader() = default;
    ~CoverDistortionShader() = default;

    /// Compiles the shader. Requires a current OpenGL context.
    void init();

    int positionLocation() const {
        return m_positionLocation;
    }
    int texcoordLocation() const {
        return m_texcoordLocation;
    }
    /// sampler2D: the cover.
    int textureLocation() const {
        return m_textureLocation;
    }
    /// sampler2D: the blurred copy of the cover.
    int blurredTextureLocation() const {
        return m_blurredTextureLocation;
    }
    /// vec2: the factors that scale the viewport to the cover.
    int fitLocation() const {
        return m_fitLocation;
    }
    /// vec2: the factors that scale the viewport to the part
    /// of the blurred copy that fills it.
    int backgroundFitLocation() const {
        return m_backgroundFitLocation;
    }
    /// float: the width of the viewport divided by its
    /// height.
    int windowAspectLocation() const {
        return m_windowAspectLocation;
    }
    /// float: the phase of the flow of the blurred copy in
    /// radians. It can wrap around at 2 pi.
    int flowPhaseLocation() const {
        return m_flowPhaseLocation;
    }
    /// float: the width of the cover divided by its height.
    int coverAspectLocation() const {
        return m_coverAspectLocation;
    }
    /// vec3: the bass, mid and high amounts, each 0..1.
    int amountsLocation() const {
        return m_amountsLocation;
    }
    /// vec2: the slow and the fast animation phase in radians.
    /// The shader only uses integer multiples of them, so they can wrap
    /// around at 2 pi.
    int phasesLocation() const {
        return m_phasesLocation;
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

    DISALLOW_COPY_AND_ASSIGN(CoverDistortionShader)
};
