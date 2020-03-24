#pragma once

#include <QList>

#include "util/color/rgbcolor.h"

class ColorPalette final {
  public:
    ColorPalette(
            QString name,
            QList<mixxx::RgbColor> colorList,
            QList<unsigned int> hotcueColorIndices = {})
            : m_name(name),
              m_colorList(colorList),
              m_hotcueColorIndices(hotcueColorIndices) {
        DEBUG_ASSERT(m_colorList.size() != 0);
    }

    mixxx::RgbColor at(int i) const {
        return m_colorList.at(i);
    }

    int size() const {
        return m_colorList.size();
    }

    int indexOf(mixxx::RgbColor color) const {
        return m_colorList.indexOf(color);
    }

    mixxx::RgbColor nextColor(mixxx::RgbColor color) const;
    mixxx::RgbColor previousColor(mixxx::RgbColor color) const;
    mixxx::RgbColor colorForHotcueIndex(unsigned int index) const;

    QList<mixxx::RgbColor>::const_iterator begin() const {
        return m_colorList.begin();
    }

    QList<mixxx::RgbColor>::const_iterator end() const {
        return m_colorList.end();
    }

    QString getName() const {
        return m_name;
    }

    void setName(const QString name) {
        m_name = name;
    }

    const QList<mixxx::RgbColor>& getColorList() const {
        return m_colorList;
    }

    QList<unsigned int> getHotcueIndices() const {
        return m_hotcueColorIndices;
    }

  private:
    QString m_name;
    QList<mixxx::RgbColor> m_colorList;
    QList<unsigned int> m_hotcueColorIndices;
};

inline bool operator==(
        const ColorPalette& lhs, const ColorPalette& rhs) {
    return lhs.getName() == rhs.getName() &&
            lhs.getColorList() == rhs.getColorList();
}
