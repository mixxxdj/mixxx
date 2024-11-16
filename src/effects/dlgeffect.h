#pragma once

#include <QDialog>
#include <QWidget>

class DlgEffect : public QDialog {
    Q_OBJECT

  public:
    DlgEffect(QWidget* customUI = nullptr);

    void setCustomUI(QWidget* customUI);

  private:
    QWidget* m_customUI;
};
