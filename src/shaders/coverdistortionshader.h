#pragma once

#include "shaders/shader.h"

namespace mixxx {
class CoverDistortionShader;
}

/// Draws a picture as a vinyl record in front of a blurred copy of the picture.
/// The record is a disc with a hole in the middle that shows the part of the
/// picture in the square around its center, shaded with grooves and with the
/// reflections of a light that stays in place while the record turns. The
/// record is warped by three independent distortions: a radial pulse for the
/// bass, a shimmer with color fringes for the mids and the glitches of a badly
/// tuned television for the highs.
/// With all amounts at zero it is drawn undistorted. The blurred copy fills the
/// viewport and flows slowly, driven by a phase that the caller advances with
/// the tempo of the music. The colors of both textures are divided by their
/// alpha channel, as CoverCombineShader requires for the pictures it produces.
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
    /// sampler2D: the picture on the record.
    int textureLocation() const {
        return m_textureLocation;
    }
    /// sampler2D: the blurred copy of the picture.
    int blurredTextureLocation() const {
        return m_blurredTextureLocation;
    }
    /// vec2: the factors that scale the viewport to the square
    /// around the record.
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
    /// vec2: the factors that scale the picture to the square
    /// around its center.
    int cropLocation() const {
        return m_cropLocation;
    }
    /// float: the angle in radians by which the record is turned
    /// clockwise. It can wrap around at 2 pi.
    int spinLocation() const {
        return m_spinLocation;
    }
    /// float: the size of a pixel of the viewport, measured in
    /// diameters of the record.
    int pixelSizeLocation() const {
        return m_pixelSizeLocation;
    }
    /// vec3: the bass, mid and high amounts, each 0..1.
    int amountsLocation() const {
        return m_amountsLocation;
    }
    /// vec2: the slow and the fast animation phase in radians.
    /// They can wrap around at 2 pi.
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
    int m_cropLocation;
    int m_spinLocation;
    int m_pixelSizeLocation;
    int m_amountsLocation;
    int m_phasesLocation;

    DISALLOW_COPY_AND_ASSIGN(CoverDistortionShader)
};
