#include "rendergraph/context.h"

#include "context_impl.h"

using namespace rendergraph;

Context::Context()
        : m_pImpl(new Context::Impl()) {
}

Context::~Context() = default;

Context::Impl& Context::impl() const {
    return *m_pImpl;
}
