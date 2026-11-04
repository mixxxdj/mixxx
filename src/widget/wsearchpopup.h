#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>

#include "preferences/usersettings.h"

class QKeyEvent;
class QShowEvent;

class WSearchPopup : public QDialog {
    Q_OBJECT
  public:
    explicit WSearchPopup(UserSettingsPointer pConfig, QWidget* parent = nullptr);
    ~WSearchPopup() override;

  signals:
    void searchRequest(const QString& result);

  protected:
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;

  private slots:
    void onSearchClicked();
    void slotHistoryActivated(int index);
    void slotFontSmaller();
    void slotFontLarger();
    void slotClearFields();
    void slotCloseOnSearchToggled(bool checked);

  private:
    QString generateQuery() const;
    void loadHistory();
    void saveHistory();
    void addToHistory(const QString& query);
    void restoreFromHistory(const QString& entry);
    void applyDarkPalette();
    void applyFontSize();

    UserSettingsPointer m_pConfig;

    QComboBox* m_historyCombo{nullptr};
    QCheckBox* m_closeOnSearchCheckBox{nullptr};

    QLineEdit* m_trackArtist{nullptr};
    QLineEdit* m_trackTitle{nullptr};
    QLineEdit* m_albumArtist{nullptr};
    QLineEdit* m_album{nullptr};
    QLineEdit* m_year{nullptr};
    QLineEdit* m_composer{nullptr};
    QLineEdit* m_key{nullptr};
    QLineEdit* m_bpm{nullptr};
    QLineEdit* m_genre{nullptr};
    QLineEdit* m_grouping{nullptr};
    QLineEdit* m_location{nullptr};
    QLineEdit* m_dateAdded{nullptr};
    QString m_bareTerm;

    QPushButton* m_searchButton{nullptr};

    QToolButton* m_fontSmallerButton{nullptr};
    QToolButton* m_fontLargerButton{nullptr};
    QPushButton* m_clearButton{nullptr};

    int m_fontSize{0};
};
