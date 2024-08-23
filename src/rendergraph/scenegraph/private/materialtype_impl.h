#pragma once

#include <QSGMaterial>

#include "rendergraph/materialtype.h"

class rendergraph::MaterialType::Impl : public QSGMaterialType {
  public:
    QSGMaterialType* sgMaterialType() {
        return this;
    }
};
