#pragma once

#include <QDialog>

class QLineEdit;
class QListWidget;
class QPushButton;

/// Dialog providing links to legal sources for free music.
class DlgFreeMusic : public QDialog {
    Q_OBJECT
  public:
    explicit DlgFreeMusic(QWidget* pParent = nullptr);

  private slots:
    void slotOpenSelectedSource();
    void slotSearchSelected();
    void slotSourceChanged();

  private:
    QListWidget* m_pSources;
    QLineEdit* m_pSearchEdit;
    QPushButton* m_pSearchButton;
};
