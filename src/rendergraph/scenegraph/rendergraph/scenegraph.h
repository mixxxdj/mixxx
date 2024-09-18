#pragma once

#include <memory>

class QQuickWindow;

namespace rendergraph {
class Context;

std::unique_ptr<Context> createSgContext(QQuickWindow* window);
} // namespace rendergraph
