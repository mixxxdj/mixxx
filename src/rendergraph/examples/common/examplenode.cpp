#include "examplenode.h"

#include <QColor>
#include <QMatrix4x4>
#include <QVector2D>

#include "rendergraph/geometry.h"
#include "rendergraph/material/rgbmaterial.h"
#include "rendergraph/material/texturematerial.h"
#include "rendergraph/material/unicolormaterial.h"
#include "rendergraph/vertexupdaters/rgbvertexupdater.h"
#include "rendergraph/vertexupdaters/texturedvertexupdater.h"
#include "rendergraph/vertexupdaters/vertexupdater.h"

using namespace rendergraph;

ExampleNode::ExampleNode(rendergraph::Context* pContext) {
    {
        TreeNode::appendChildNode(std::make_unique<GeometryNode>());
        auto pNode = static_cast<GeometryNode*>(TreeNode::lastChild());
        pNode->initForRectangles<TextureMaterial>(1);
        auto& material = dynamic_cast<TextureMaterial&>(pNode->material());
        material.setTexture(std::make_unique<Texture>(
                pContext, QImage(":/example/images/test.png")));
        TexturedVertexUpdater vertexUpdater{
                pNode->geometry().vertexDataAs<Geometry::TexturedPoint2D>()};
        vertexUpdater.addRectangle({0, 0}, {100, 100}, {0.f, 0.f}, {1.f, 1.f});
    }
    {
        TreeNode::appendChildNode(std::make_unique<GeometryNode>());
        auto pNode = static_cast<GeometryNode*>(TreeNode::lastChild());
        pNode->initForRectangles<rendergraph::UniColorMaterial>(2);
        pNode->material().setUniform(1, QColor(255, 127, 0));
        pNode->geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
        rendergraph::VertexUpdater vertexUpdater{
                pNode->geometry()
                        .vertexDataAs<rendergraph::Geometry::Point2D>()};
        vertexUpdater.addRectangle({100, 100}, {160, 160});
        vertexUpdater.addRectangle({200, 160}, {240, 190});
    }
    {
        TreeNode::appendChildNode(std::make_unique<GeometryNode>());
        auto pNode = static_cast<GeometryNode*>(TreeNode::lastChild());
        pNode->initForRectangles<rendergraph::RGBMaterial>(2);
        pNode->geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
        rendergraph::RGBVertexUpdater vertexUpdater{
                pNode->geometry().vertexDataAs<Geometry::RGBColoredPoint2D>()};
        vertexUpdater.addRectangle({300, 100}, {340, 140}, {1.f, 0.f, 0.5f});
        vertexUpdater.addRectangleHGradient(
                {340, 100}, {440, 130}, {0.f, 1.f, 0.5f}, {0.5f, 0.f, 1.f});
    }
}
