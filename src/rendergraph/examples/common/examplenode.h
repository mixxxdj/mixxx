#include "rendergraph/geometrynode.h"
#include "rendergraph/node.h"
#include "rendergraph/texture.h"

namespace rendergraph {
class ExampleNode;
} // namespace rendergraph

class rendergraph::ExampleNode : public rendergraph::Node {
  public:
    ExampleNode(rendergraph::Context* pContext);
};
