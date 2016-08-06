#ifndef WRIGHTCLICKBUTTON_H
#define WRIGHTCLICKBUTTON_H

#include <QToolButton>
#include <QMouseEvent>
#include <QBasicTimer>

#include "library/libraryfeature.h"
#include "control/controlproxy.h"

class WFeatureClickButton : public QToolButton
{
    Q_OBJECT
    
public:
    WFeatureClickButton(LibraryFeature* pFeature = nullptr, 
                        QWidget* parent = nullptr);
    
    void setText(const QString& text);
    
signals:
    
    void clicked(LibraryFeature*);
    void rightClicked(const QPoint&);
    void hoverShow(LibraryFeature*);
    
protected:
    
    void mousePressEvent(QMouseEvent* event);
    
    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent*);
    void dropEvent(QDropEvent* event);
    
    void timerEvent(QTimerEvent* event);
    
private slots:
    
    void slotClicked();
    void slotTextDisplayChanged(double value);
    
private:
    
    static const int kHoverTime;
    
    ControlProxy m_textControl;
    LibraryFeature* m_pFeature;
    QBasicTimer m_hoverTimer;
};

#endif // WRIGHTCLICKBUTTON_H
