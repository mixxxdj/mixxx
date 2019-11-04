//
// Created by Ferran Pujol Camins on 27/10/2019.
//

#pragma once

#include <QColor>
#include "QList"

class ColorPalette {
  public:
    ColorPalette(QList<QColor>);

    static const ColorPalette mixxxHotcuesPalette;

    QList<QColor> m_colorList;
};

inline bool operator==(const ColorPalette& lhs, const ColorPalette& rhs) {
    return lhs.m_colorList == rhs.m_colorList;
}
