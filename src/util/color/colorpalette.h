#pragma once

#include <QList>

#include "util/color/rgbcolor.h"

// An ordered list of colors that can be picked by the user from WColorPicker,
// used for cue and track colors. Also used by CueControl to map default
// colors to hotcues based on their hotcue number
class ColorPalette final {
  public:
    ColorPalette(
            const QString& name,
            const QList<mixxx::RgbColor>& colorList,
            const QList<int>& colorIndicesByHotcue = {})
            : m_name(name),
              m_colorList(colorList),
              m_colorIndicesByHotcue(colorIndicesByHotcue) {
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
    mixxx::RgbColor::optional_t nextColor(mixxx::RgbColor::optional_t color) const;
    mixxx::RgbColor previousColor(mixxx::RgbColor color) const;
    mixxx::RgbColor::optional_t previousColor(mixxx::RgbColor::optional_t color) const;
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

    void setName(const QString& name) {
        m_name = name;
    }

    const QList<mixxx::RgbColor>& getColorList() const {
        return m_colorList;
    }

    QList<int> getIndicesByHotcue() const {
        return m_colorIndicesByHotcue;
    }

  private:
    QString m_name;
    QList<mixxx::RgbColor> m_colorList;
    QList<int> m_colorIndicesByHotcue;
};

inline bool operator==(
        const ColorPalette& lhs, const ColorPalette& rhs) {
    return lhs.getName() == rhs.getName() &&
            lhs.getColorList() == rhs.getColorList();
}
