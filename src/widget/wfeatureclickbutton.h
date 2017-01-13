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
    WFeatureClickButton(LibraryFeature* pFeature, QWidget* parent);

  signals:

    void clicked(LibraryFeature*);
    void rightClicked(const QPoint&);
    void hoverShow(LibraryFeature*);
    
    void hovered(LibraryFeature*);
    void leaved(LibraryFeature*);
    void focusIn(LibraryFeature*);
    void focusOut(LibraryFeature*);

  protected:

    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent * event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent*) override;
    void dropEvent(QDropEvent* event) override;

    void timerEvent(QTimerEvent* event) override;


  private slots:

    void slotClicked();
    void slotTextDisplayChanged(double value);

  private:

    static const int kHoverTime;

    ControlProxy m_textControl;
    LibraryFeature* m_pFeature;
    QBasicTimer m_hoverTimer;
    bool m_mousEntered;
};

#endif // WRIGHTCLICKBUTTON_H
