#ifndef WTRISTATEBUTTON_H
#define WTRISTATEBUTTON_H

#include <QToolButton>

class WTriStateButton : public QToolButton
{
  public:
    enum State {
        Unactive,
        Active,
        Hovered
    };
    
    WTriStateButton(QWidget* parent = nullptr);

    void setChecked(bool value);
    bool isChecked() const;
    
    void setState(State state);
    State getState() const;
    
    void setHovered(bool value);
    
    void setIcon(const QIcon& icon);
    
  private:
    void updateButton();
    
    QIcon m_icon;
    State m_state;
};

#endif // WTRISTATEBUTTON_H
