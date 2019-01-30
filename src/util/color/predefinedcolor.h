#ifndef PREDEFINEDCOLOR_H
#define PREDEFINEDCOLOR_H

#include <QColor>

class PredefinedColor final {
  public:
    PredefinedColor(QColor defaultRepresentation, QString sName, QString sDisplayName);

    const QColor m_defaultRepresentation;
    const QString m_sName;
    const QString m_sDisplayName;
    const int m_iBrightness;
};
typedef std::shared_ptr<const PredefinedColor> PredefinedColorPointer;

#endif /* PREDEFINEDCOLOR_H */
