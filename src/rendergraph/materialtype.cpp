#include "rendergraph/materialtype.h"

#include "materialtype_impl.h"

using namespace rendergraph;

MaterialType::MaterialType(Impl* pImpl)
        : m_pImpl(pImpl) {
}

MaterialType::MaterialType()
        : MaterialType(new MaterialType::Impl()){};

MaterialType::~MaterialType() = default;

MaterialType::Impl& MaterialType::impl() const {
    return *m_pImpl;
}
