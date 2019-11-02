//
// Created by Ferran Pujol Camins on 27/10/2019.
//

#pragma once

#include <QColor>
#include <QList>

class HotcueColorPalette {
  public:
    HotcueColorPalette(QList<QColor> colorList)
            : m_colorList(colorList) {
    }

    QColor at(int i) const {
        return m_colorList.at(i);
    }

    int size() const {
        return m_colorList.size();
    }

    int indexOf(QColor color) const {
        return m_colorList.indexOf(color);
    }

    QList<QColor>::const_iterator begin() const {
        return m_colorList.begin();
    }

    QList<QColor>::const_iterator end() const {
        return m_colorList.end();
    }

    static const HotcueColorPalette mixxxPalette;

    QList<QColor> m_colorList;
};

inline bool operator==(
        const HotcueColorPalette& lhs, const HotcueColorPalette& rhs) {
    return lhs.m_colorList == rhs.m_colorList;
}
