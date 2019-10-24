#pragma once

#include <QColor>
#include <QObject>

#include "predefinedcolorsrepresentation.h"
#include "util/color/predefinedcolor.h"

// This class defines a set of predefined colors and provides some handy functions to work with them.
// A single global instance is create in the Color namespace.
// This class is thread-safe because all its methods and public properties are const.
class PredefinedColorsSet final {
  public:
    const PredefinedColorPointer noColor = std::make_shared<PredefinedColor>(
            QColor(),
            QLatin1String("No Color"),
            QObject::tr("No Color"),
            0);
    const PredefinedColorPointer red = std::make_shared<PredefinedColor>(
            QColor("#c50a08"),
            QLatin1String("Red"),
            QObject::tr("Red"),
            1);
    const PredefinedColorPointer green = std::make_shared<PredefinedColor>(
            QColor("#32be44"),
            QLatin1String("Green"),
            QObject::tr("Green"),
            2);
    const PredefinedColorPointer blue = std::make_shared<PredefinedColor>(
            QColor("#0044ff"),
            QLatin1String("Blue"),
            QObject::tr("Blue"),
            3);
    const PredefinedColorPointer yellow = std::make_shared<PredefinedColor>(
            QColor("#f8d200"),
            QLatin1String("Yellow"),
            QObject::tr("Yellow"),
            4);
    const PredefinedColorPointer cyan = std::make_shared<PredefinedColor>(
            QColor("#42d4f4"),
            QLatin1String("Celeste"),
            QObject::tr("Celeste"),
            5);
    const PredefinedColorPointer magenta = std::make_shared<PredefinedColor>(
            QColor("#af00cc"),
            QLatin1String("Purple"),
            QObject::tr("Purple"),
            6);
    const PredefinedColorPointer pink = std::make_shared<PredefinedColor>(
            QColor("#fca6d7"),
            QLatin1String("Pink"),
            QObject::tr("Pink"),
            7);
    const PredefinedColorPointer white = std::make_shared<PredefinedColor>(
            QColor("#f2f2ff"),
            QLatin1String("White"),
            QObject::tr("White"),
            8);

    // The list of the predefined colors.
    const QList<PredefinedColorPointer> allColors{
            noColor,
            red,
            green,
            yellow,
            blue,
            cyan,
            magenta,
            pink,
            white,
    };

    PredefinedColorsSet()
            : m_defaultRepresentation() {
        for (PredefinedColorPointer color : allColors) {
            m_defaultRepresentation.setCustomRgba(color, color->m_defaultRgba);
        }
    }

    // Returns the position of a PredefinedColor in the allColors list.
    int predefinedColorIndex(PredefinedColorPointer searchedColor) const {
        for (int position = 0; position < allColors.count(); ++position) {
            PredefinedColorPointer color(allColors.at(position));
            if (*color == *searchedColor) {
                return position;
            }
        }
        return 0;
    };

    // Return a predefined color from its name.
    // Return noColor if there's no color with such name.
    PredefinedColorPointer predefinedColorFromName(QString name) const {
        for (PredefinedColorPointer color : allColors) {
            if (color->m_sName == name) {
                return color;
            }
        }
        return noColor;
    };

    // Return a predefined color from its id.
    // Return noColor if there's no color with iId.
    PredefinedColorPointer predefinedColorFromId(int iId) const {
        for (PredefinedColorPointer color : allColors) {
            if (color->m_iId == iId) {
                return color;
            }
        }
        return noColor;
    };

    // The default color representation, i.e. maps each predefined color to its default Rgba.
    PredefinedColorsRepresentation defaultRepresentation() const {
        return m_defaultRepresentation;
    };

  private:
    PredefinedColorsRepresentation m_defaultRepresentation;
};
