#pragma once

#include <QDialog>
#include <QSqlDatabase>

#include "library/importentry.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTextStream;

class WDlgImportPlaylist : public QDialog {
    Q_OBJECT
  public:
    WDlgImportPlaylist(const QList<ImportEntry>& entries,
            const QSqlDatabase& database,
            int playlistId,
            QTextStream* reportStream,
            QWidget* parent = nullptr);

    static std::optional<QList<ImportEntry>> parseImportFile(
            const QString& path, QString* errorMessage = nullptr);

  private:
    void advanceToNext();
    void runSearch(const QString& title, const QString& artist);
    void addSelectedTracks();

    QList<ImportEntry> m_entries;
    QSqlDatabase m_database;
    int m_playlistId;
    QTextStream* m_reportStream;

    int m_currentIndex{0};
    int m_position{0};
    bool m_importedThisEntry{false};

    QLabel* m_labelCurrentEntry{nullptr};
    QLineEdit* m_lineEditTitle{nullptr};
    QLineEdit* m_lineEditArtist{nullptr};
    QPushButton* m_searchButton{nullptr};
    QCheckBox* m_checkMatchBoth{nullptr};
    QTableWidget* m_tableCandidates{nullptr};
    QPushButton* m_nextButton{nullptr};
    QPushButton* m_addSelectedButton{nullptr};
    QPushButton* m_cancelButton{nullptr};

    friend class BasePlaylistFeature;
};
