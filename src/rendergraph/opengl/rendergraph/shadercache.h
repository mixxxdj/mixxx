#pragma once

#include <map>

#include "rendergraph/material.h"
#include "rendergraph/materialshader.h"

namespace rendergraph {
class ShaderCache;
}

class rendergraph::ShaderCache {
  private:
    static std::map<MaterialType*, std::shared_ptr<MaterialShader>>& map() {
        static std::map<MaterialType*, std::shared_ptr<MaterialShader>> s_map;
        return s_map;
    }

  public:
    static std::shared_ptr<MaterialShader> getShaderForMaterial(Material* pMaterial) {
        auto iter = map().find(pMaterial->type());
        if (iter != map().end()) {
            return iter->second;
        }
        auto pResult = std::shared_ptr<MaterialShader>(pMaterial->createShader().release());
        map().insert(std::pair<MaterialType*, std::shared_ptr<MaterialShader>>{
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
