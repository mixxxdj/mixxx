#ifndef WRIGHTCLICKBUTTON_H
#define WRIGHTCLICKBUTTON_H

#include <QToolButton>
#include <QMouseEvent>

class WRightClickButton : public QToolButton
{
    Q_OBJECT
    
public:
    WRightClickButton(QWidget* parent = nullptr);
    
signals:
    
    void rightClicked(const QPoint&);
    
protected:
    
    void mousePressEvent(QMouseEvent* event);
};

#endif // WRIGHTCLICKBUTTON_H
