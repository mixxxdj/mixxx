#include "rendergraph/uniformscache.h"

#include "rendergraph/uniformset.h"

using namespace rendergraph;

UniformsCache::UniformsCache(const UniformSet& uniformSet) {
    int offset = 0;
    m_infos.reserve(uniformSet.uniforms().size());
    for (const auto& uniform : uniformSet.uniforms()) {
        const int size = sizeOf(uniform.m_type);
        m_infos.push_back({uniform.m_type, offset});
        offset += size;
    }
    m_byteArray.resize(offset);
    m_byteArray.fill('\0');
}

UniformsCache::~UniformsCache() = default;

void UniformsCache::set(int uniformIndex, const void* ptr, int size) {
    memcpy(m_byteArray.data() + m_infos[uniformIndex].m_offset, ptr, size);
}

void UniformsCache::get(int uniformIndex, void* ptr, int size) const {
    memcpy(ptr, m_byteArray.data() + m_infos[uniformIndex].m_offset, size);
}
