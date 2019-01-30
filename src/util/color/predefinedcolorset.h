#ifndef COLOR_H
#define COLOR_H

#include <QColor>

#include "util/color/predefinedcolor.h"
#include "util/color/predefinedcolormap.h"

// These methods and properties are not thread-safe, use them only on the GUI thread
class PredefinedColorSet final {
  public:
    const PredefinedColorPointer red = std::make_shared<PredefinedColor>(
        QColor("#E6194B"),
        QLatin1String("Red"),
        QObject::tr("Red")
    );
    const PredefinedColorPointer green = std::make_shared<PredefinedColor>(
        QColor("#3CB44B"),
        QLatin1String("Green"),
        QObject::tr("Green")
    );
    const PredefinedColorPointer yellow = std::make_shared<PredefinedColor>(
        QColor("#FFE119"),
        QLatin1String("Yellow"),
        QObject::tr("Yellow")
    );
    const PredefinedColorPointer blue = std::make_shared<PredefinedColor>(
        QColor("#4363D8"),
        QLatin1String("Blue"),
        QObject::tr("Blue")
    );
    const PredefinedColorPointer cyan = std::make_shared<PredefinedColor>(
        QColor("#42D4F4"),
        QLatin1String("Cyan"),
        QObject::tr("Cyan")
    );
    const PredefinedColorPointer magenta = std::make_shared<PredefinedColor>(
        QColor("#F032E6"),
        QLatin1String("Magenta"),
        QObject::tr("Magenta")
    );
    const PredefinedColorPointer pink = std::make_shared<PredefinedColor>(
        QColor("#FABEBE"),
        QLatin1String("Pink"),
        QObject::tr("Pink")
    );
    const PredefinedColorPointer teal = std::make_shared<PredefinedColor>(
        QColor("#469990"),
        QLatin1String("Teal"),
        QObject::tr("Teal")
    );
    const PredefinedColorPointer grey = std::make_shared<PredefinedColor>(
        QColor("#A9A9A9"),
        QLatin1String("Grey"),
        QObject::tr("Grey")
    );
    const PredefinedColorPointer invalid = std::make_shared<PredefinedColor>(
        QColor(),
        QLatin1String("Invalid Color"),
        QObject::tr("Invalid Color")
    );

    // The list of the predefined colors.
    const QList<PredefinedColorPointer> allColors {
        red,
        green,
        yellow,
        blue,
        cyan,
        magenta,
        pink,
        teal,
        grey,
    };

    PredefinedColorSet();

    // A list with the internal names of the predefined colors.
    QList<QString> predefinedColorNames() const { return m_predefinedColorsNames; };

    // Return a predefined color from its internal name.
    PredefinedColorPointer predefinedColorFromName(QString name) const {
        for (PredefinedColorPointer color : allColors) {
            if (color->m_sName == name) {
                return color;
            }
        }
        return invalid;
    };

    // The default color map, i.e. maps each predefined color to itself.
    //
    // It's fast to copy the default representation. See comment on ColorsRepresentation.
    PredefinedColorMap defaultMap() const { return m_defaultMap; };

  private:
    QList<QString> m_predefinedColorsNames;
    PredefinedColorMap m_defaultMap;
};
#endif /* COLOR_H */
