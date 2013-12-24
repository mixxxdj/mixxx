#ifndef WWIDGETGROUP_H
#define WWIDGETGROUP_H

#include <QDomNode>
#include <QGroupBox>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QString>
#include <QWidget>

#include "widget/wpixmapstore.h"

class WWidgetGroup : public QGroupBox {
    Q_OBJECT
  public:
    WWidgetGroup(QWidget* pParent=NULL);
    virtual ~WWidgetGroup();

    void setup(QDomNode node);
    void setPixmapBackground(const QString &filename);
    void addWidget(QWidget* pChild);

  protected:
    virtual void paintEvent(QPaintEvent* pe);
    virtual void resizeEvent(QResizeEvent* re);

  private:
    // Associated background pixmap
    QPixmapPointer m_pPixmapBack;
    QPixmap m_pixmapBackScaled;
};

#endif // WWIDGETGROUP_H
