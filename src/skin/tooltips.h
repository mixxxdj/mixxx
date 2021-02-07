#pragma once

#include <QHash>
#include <QStringList>
#include <QObject>

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
