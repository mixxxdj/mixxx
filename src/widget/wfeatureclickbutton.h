#ifndef WRIGHTCLICKBUTTON_H
#define WRIGHTCLICKBUTTON_H

#include <library/libraryfeature.h>
#include <QToolButton>
#include <QMouseEvent>

class WFeatureClickButton : public QToolButton
{
    Q_OBJECT
    
public:
    WFeatureClickButton(LibraryFeature* feature = nullptr, 
                        QWidget* parent = nullptr);
    
    void setData(const QString& data);
    
signals:
    
    void clicked(const QString& view);
    
    void rightClicked(const QPoint&);
    
    void hoverShow(const QString& feature);
    
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
    
    QString m_data;
    
    LibraryFeature* m_feature;
    
    QBasicTimer m_hoverTimer;
};

#endif // WRIGHTCLICKBUTTON_H
