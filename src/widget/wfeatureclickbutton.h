#ifndef WRIGHTCLICKBUTTON_H
#define WRIGHTCLICKBUTTON_H

#include <QToolButton>
#include <QMouseEvent>

class WFeatureClickButton : public QToolButton
{
    Q_OBJECT
    
public:
    WFeatureClickButton(QWidget* parent = nullptr);
    
    void setData(const QString& data);
    
signals:
    
    void clicked(const QString& view);
    
    void rightClicked(const QPoint&);
    
protected:
    
    void mousePressEvent(QMouseEvent* event);
    
    void dropEvent(QDropEvent* event);
    
private slots:
    
    void slotClicked();
    
private:
    
    QString m_data;
};

#endif // WRIGHTCLICKBUTTON_H
