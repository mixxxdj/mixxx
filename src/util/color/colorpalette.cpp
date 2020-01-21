//
// Created by Ferran Pujol Camins on 27/10/2019.
//

#include "colorpalette.h"

// static 
const ColorPalette ColorPalette::mixxxHotcuesPalette =
        ColorPalette(QList<QRgb>{
                0xc50a08,
                0x32be44,
                0x0044ff,
                0xf8d200,
                0x42d4f4,
                0xaf00cc,
                0xfca6d7,
                0xf2f2ff});

ColorPalette::ColorPalette(QList<QRgb> colorList)
    : m_colorList(colorList) {
}


