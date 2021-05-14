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
    QString tooltipForTemplate(const QString& id) const;


  private:
    void addStandardTooltips();
    QString tooltipSeparator() const;
    QList<QString>& add(const QString& id);

    QHash<QString, QStringList> m_tooltips;
    QHash<QString, QString> m_headlines;
    QHash<QString, QString> m_description;

    QString m_dropTracksHere;
    QString m_resetToDefault;
    QString m_leftClick;
    QString m_rightClick;
    QString m_doubleClick;
    QString m_scrollWheel;
    QString m_scratchMouse;
    QString m_shift;
};
