#pragma once

#include "shaders/shader.h"

namespace mixxx {
class CoverCombineShader;
}

/// [AI-generated] Draws the part of a cover image that lies inside one cell
/// of the viewport. The cell is bounded by up to three straight borders.
/// Drawing the covers of several decks with the cells of a layout, and
/// adding them up, combines them into a single picture. The alpha channel
/// of the result tells how much has been drawn at a position, so that the
/// colors can be divided by it where soft borders overlap. [/AI-generated]
class mixxx::CoverCombineShader final : public mixxx::Shader {
  public:
    CoverCombineShader() = default;
    ~CoverCombineShader() = default;

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
    /// [AI-generated] vec3[3]: the borders of the cell. For a position p in
    /// the viewport, which spans -0.5 to 0.5 on both axes, dot(xy, p) + z is
    /// the distance from the border, positive inside of the cell.
    /// [/AI-generated]
    int bordersLocation() const {
        return m_bordersLocation;
    }
    /// [AI-generated] float: the reciprocal of the distance over which a
    /// border fades from inside to outside. [/AI-generated]
    int sharpnessLocation() const {
        return m_sharpnessLocation;
    }
    /// [AI-generated] vec2: the factors that scale the viewport to the part
    /// of the cover that fills it. [/AI-generated]
    int fitLocation() const {
        return m_fitLocation;
    }

  private:
    int m_positionLocation;
    int m_texcoordLocation;
    int m_textureLocation;
    int m_bordersLocation;
    int m_sharpnessLocation;
    int m_fitLocation;

    DISALLOW_COPY_AND_ASSIGN(CoverCombineShader)
};
