#pragma once

#include <QDialog>

class QListWidget;

/// Dialog providing links to legal sources for free music.
class DlgFreeMusic : public QDialog {
    Q_OBJECT
  public:
    explicit DlgFreeMusic(QWidget* pParent = nullptr);

  private slots:
    void slotOpenSelectedSource();

  private:
    QListWidget* m_pSources;
};
