//
// Created by Ferran Pujol Camins on 27/10/2019.
//

#include "hotcuecolorpalette.h"

HotcueColorPalette::HotcueColorPalette(QList<QColor> colorList)
        : m_colorList(colorList) {
}

const HotcueColorPalette HotcueColorPalette::mixxxPalette =
        HotcueColorPalette(QList<QColor>{QColor("#c50a08"),
                QColor("#32be44"),
                QColor("#0044ff"),
                QColor("#f8d200"),
                QColor("#42d4f4"),
                QColor("#af00cc"),
                QColor("#fca6d7"),
                QColor("#f2f2ff")});
