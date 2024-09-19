#include "rendergraph/geometrynode.h"
#include "rendergraph/node.h"
#include "rendergraph/texture.h"

namespace rendergraph {
class ExampleNode1;
class ExampleNode2;
class ExampleNode3;
class ExampleTopNode;
} // namespace rendergraph

class rendergraph::ExampleNode1 : public rendergraph::GeometryNode {
  public:
    static constexpr float positionArray[] = {-1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f};
    static constexpr float verticalGradientArray[] = {1.f, 1.f, -1.f, -1.f};
    static constexpr float horizontalGradientArray[] = {-1.f, 1.f, -1.f, 1.f};

    ExampleNode1();
};

class rendergraph::ExampleNode2 : public rendergraph::GeometryNode {
  public:
    static constexpr float positionArray[] = {0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f};
    static constexpr float verticalGradientArray[] = {1.f, 1.f, 0.f, 0.f};
    static constexpr float horizontalGradientArray[] = {-1.f, 1.f, -1.f, 1.f};

    ExampleNode2();
};

class rendergraph::ExampleNode3 : public rendergraph::GeometryNode {
  public:
    static constexpr float positionArray[] = {-1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f};
    static constexpr float texcoordArray[] = {0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f};

    ExampleNode3();

    void setTexture(std::unique_ptr<Texture> texture);
};

class rendergraph::ExampleTopNode : public rendergraph::Node {
  public:
    ExampleTopNode(rendergraph::Context& context);
};
