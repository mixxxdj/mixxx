#ifndef WRIGHTCLICKBUTTON_H
#define WRIGHTCLICKBUTTON_H

#include <library/libraryfeature.h>
#include <QToolButton>
#include <QMouseEvent>
#include <QBasicTimer>

class WFeatureClickButton : public QToolButton
{
    Q_OBJECT
    
public:
    WFeatureClickButton(LibraryFeature* pFeature = nullptr, 
                        QWidget* parent = nullptr);
    
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
    
private:
    
    static const int kHoverTime;
    
    LibraryFeature* m_pFeature;
    QBasicTimer m_hoverTimer;
};

#endif // WRIGHTCLICKBUTTON_H
