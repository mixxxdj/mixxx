#ifndef TOOLTIPS_H
#define TOOLTIPS_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtCore>

class Tooltips : public QObject {
    Q_OBJECT
  public:
    Tooltips();
    virtual ~Tooltips();
    QString tooltipForId(const QString& id) const;

  private:
    void addStandardTooltips();
    QString tooltipSeparator() const;
    QList<QString>& add(const QString& id);

    QHash<QString, QStringList> m_tooltips;
};


#endif /* TOOLTIPS_H */
