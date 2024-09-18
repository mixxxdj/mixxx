#include "scenegraph.h"

#include "rendergraph/context.h"
#include "rendergraph/node.h"

using namespace rendergraph;

std::unique_ptr<Context> rendergraph::createSgContext(QQuickWindow* window) {
    auto context = std::make_unique<Context>();
    context->setWindow(window);
    return context;
}
