#include "examplenodes.h"

#include <QColor>
#include <QMatrix4x4>
#include <QVector2D>

#include "endoftrackmaterial.h"
#include "rendergraph/geometry.h"
#include "rendergraph/material/texturematerial.h"

using namespace rendergraph;

ExampleNode1::ExampleNode1() {
    setMaterial(std::make_unique<EndOfTrackMaterial>());
    setGeometry(std::make_unique<Geometry>(EndOfTrackMaterial::attributes(), 4));

    geometry().setAttributeValues(0, positionArray, 4);
    geometry().setAttributeValues(1, horizontalGradientArray, 4);

    QColor color("red");
    material().setUniform(0,
            QVector4D{color.redF(),
                    color.greenF(),
                    color.blueF(),
                    color.alphaF()});
}

ExampleNode2::ExampleNode2() {
    setMaterial(std::make_unique<EndOfTrackMaterial>());
    setGeometry(std::make_unique<Geometry>(EndOfTrackMaterial::attributes(), 4));

    geometry().setAttributeValues(0, positionArray, 4);
    geometry().setAttributeValues(1, horizontalGradientArray, 4);

    QColor color("blue");
    material().setUniform(0,
            QVector4D{color.redF(),
                    color.greenF(),
                    color.blueF(),
                    color.alphaF()});
}

ExampleNode3::ExampleNode3() {
    auto* m = new TextureMaterial;

    setMaterial(std::make_unique<TextureMaterial>());
    setGeometry(std::make_unique<Geometry>(TextureMaterial::attributes(), 4));

    geometry().setAttributeValues(0, positionArray, 4);
    geometry().setAttributeValues(1, texcoordArray, 4);

    QMatrix4x4 matrix;

    matrix.scale(0.3);
    material().setUniform(0, matrix);
    material().setUniform(1, QVector4D{1.f, 0.5f, 0.2f, 0.7f});
}

void ExampleNode3::setTexture(std::unique_ptr<Texture> texture) {
    dynamic_cast<TextureMaterial&>(material()).setTexture(std::move(texture));
}
