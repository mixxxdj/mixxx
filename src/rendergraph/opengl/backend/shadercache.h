#pragma once

#include <map>

#include "rendergraph/material.h"
#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"

namespace rendergraph::backend {
class ShaderCache;
}

class rendergraph::backend::ShaderCache {
  private:
    static std::map<rendergraph::MaterialType*,
            std::shared_ptr<rendergraph::MaterialShader>>&
    map() {
        static std::map<rendergraph::MaterialType*,
                std::shared_ptr<rendergraph::MaterialShader>>
                s_map;
        return s_map;
    }

  public:
    static std::shared_ptr<rendergraph::MaterialShader> getShaderForMaterial(
            rendergraph::Material* pMaterial) {
        auto iter = map().find(pMaterial->type());
        if (iter != map().end()) {
            return iter->second;
        }
        auto pResult = std::shared_ptr<rendergraph::MaterialShader>(
                pMaterial->createShader().release());
        map().insert(std::pair<rendergraph::MaterialType*,
                std::shared_ptr<rendergraph::MaterialShader>>{
                pMaterial->type(), pResult});
        return pResult;
    }
    static void purge() {
        auto iter = map().begin();
        while (iter != map().end()) {
            if (iter->second.use_count() == 1) {
                iter = map().erase(iter);
            } else {
                ++iter;
            }
        }
    }
};
