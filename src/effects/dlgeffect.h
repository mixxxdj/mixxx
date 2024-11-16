#pragma once

#include <QCloseEvent>
#include <QDialog>
#include <QWidget>

class DlgEffect : public QDialog {
    Q_OBJECT

  public:
    DlgEffect(QWidget* customUI = nullptr);
    virtual ~DlgEffect();

    void setCustomUI(QWidget* customUI);

  signals:
    void closed();

  protected:
    void closeEvent(QCloseEvent* e) override;

  private:
    QWidget* m_customUI;
};
