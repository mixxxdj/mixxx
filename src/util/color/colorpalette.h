#pragma once

#include <QColor>
#include <QList>

class ColorPalette {
  public:
    ColorPalette(QList<QColor> colorList)
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

    static const ColorPalette mixxxPalette;

    QList<QColor> m_colorList;
};

inline bool operator==(
        const ColorPalette& lhs, const ColorPalette& rhs) {
    return lhs.m_colorList == rhs.m_colorList;
}
