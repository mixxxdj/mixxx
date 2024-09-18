#pragma once

#include <QSGMaterial>

namespace rendergraph {
class MaterialType;
}

class rendergraph::MaterialType : public QSGMaterialType {
  public:
    MaterialType();
    ~MaterialType();
};
