#pragma once

#include <unordered_map>

#include "rendergraph/material.h"
#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"

namespace rendergraph {
class ShaderCache;
} // namespace rendergraph

class rendergraph::ShaderCache {
  private:
    static std::unordered_map<MaterialType*,
            std::shared_ptr<MaterialShader>>&
    map() {
        static std::unordered_map<MaterialType*,
                std::shared_ptr<MaterialShader>>
                s_map;
        return s_map;
    }

  public:
    static std::shared_ptr<MaterialShader> getShaderForMaterial(
            Material* pMaterial) {
        auto iter = map().find(pMaterial->type());
        if (iter != map().end()) {
            return iter->second;
        }
        auto pResult = std::shared_ptr<MaterialShader>(
                pMaterial->createShader());
        map().insert(std::pair<MaterialType*,
                std::shared_ptr<MaterialShader>>{
                pMaterial->type(), pResult});
        return pResult;
    }
    static void purge() {
        std::erase_if(map(), [](const auto& item) {
            auto const& [key, value] = item;
            return value.use_count() == 1;
        });
    }
};
