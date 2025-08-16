#pragma once

#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"

namespace rendergraph {
class BaseMaterial;
}

class rendergraph::BaseMaterial {
  protected:
    BaseMaterial() = default;

  public:
    virtual MaterialType* type() const = 0;

    // For parity with QSGMaterial, not used yet.
    int compare(const BaseMaterial* other) const;

    void setShader(std::shared_ptr<MaterialShader> pShader);

    MaterialShader& shader() const;

    int uniformLocation(int uniformIndex) const;
    int attributeLocation(int attributeIndex) const;

    void modifyShader();
    bool isLastModifierOfShader() const;

  private:
    std::shared_ptr<MaterialShader> m_pShader;
};
