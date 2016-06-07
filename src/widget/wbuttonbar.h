#ifndef WBUTTONBAR_H
#define WBUTTONBAR_H

#include <QLayout>
#include <QVariant>
#include <QIcon>
#include <QPushButton>
#include <QButtonGroup>

#include "widget/wwidget.h"

class WButtonBar : public WWidget
{
    Q_OBJECT
  public:
    WButtonBar(QWidget* parent = nullptr);

    void addButton(const QIcon& icon, const QVariant& title, const QString& data);

  signals:

    void buttonClicked(const QString&);

  private slots:

    void slotButtonClicked(int id);

  private:

    QLayout* m_pLayout;
    QButtonGroup* m_pButtonGroup;
    QList<QString> m_data;
};

#endif // WBUTTONBAR_H
