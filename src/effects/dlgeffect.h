#pragma once

#include <QDialog>
#include <QWidget>

class DlgEffect : public QDialog {
    Q_OBJECT

  public:
    DlgEffect(QWidget* customUI = nullptr);
    virtual ~DlgEffect();

    void setCustomUI(QWidget* customUI);

  private:
    QWidget* m_customUI;
};
