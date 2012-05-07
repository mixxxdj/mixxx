#ifndef TOOLTIPS_H
#define TOOLTIPS_H

#include <QHash>
#include <QStringList>
#include <QObject>

class Tooltips : public QObject {
    Q_OBJECT
  public:
    Tooltips();
    virtual ~Tooltips();
    QString tooltipForId(QString id) const;

  private:
    void addStandardTooltips();
    QString tooltipSeparator() const;
    QList<QString>& add(QString id);

    QHash<QString, QStringList> m_tooltips;
};


#endif /* TOOLTIPS_H */
