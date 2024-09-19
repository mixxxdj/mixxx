#pragma once

#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"

namespace rendergraph::backend {
class Material;
}

class rendergraph::backend::Material {
  protected:
    Material() = default;

  public:
    virtual rendergraph::MaterialType* type() const = 0;

    void setShader(std::shared_ptr<rendergraph::MaterialShader> pShader);

    rendergraph::MaterialShader& shader() const;

    int uniformLocation(int uniformIndex) const;
    int attributeLocation(int attributeIndex) const;

    void modifyShader();
    bool isLastModifierOfShader() const;
    std::shared_ptr<rendergraph::MaterialShader> m_pShader;
};
