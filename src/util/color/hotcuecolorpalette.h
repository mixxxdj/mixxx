//
// Created by Ferran Pujol Camins on 27/10/2019.
//

#pragma once

#include <QColor>
#include "QList"

class HotcueColorPalette {
  public:
    HotcueColorPalette(QList<QColor>);

    static const HotcueColorPalette mixxxPalette;

    QList<QColor> m_colorList;
};