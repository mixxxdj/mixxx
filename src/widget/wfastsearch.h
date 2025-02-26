#ifndef WFASTSEARCH_H
#define WFASTSEARCH_H

#include <QDialog>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>

class WFastSearch : public QDialog {
    Q_OBJECT

  public:
    explicit WFastSearch(QWidget* parent = nullptr);
    ~WFastSearch();

  signals:
    void searchRequest(const QString& query);
    void search2CrateRequest(const QString& query);

  protected:
    void keyPressEvent(QKeyEvent* event) override;

  private slots:
    void onSearchClicked();
    void onSearch2CrateClicked();

  private:
    QLineEdit* m_trackTitle;
    QLineEdit* m_trackArtist;
    QLineEdit* m_albumArtist;
    QLineEdit* m_album;
    QLineEdit* m_year;
    QLineEdit* m_composer;
    QLineEdit* m_key;
    QLineEdit* m_bpm;

    QPushButton* m_searchButton;
    QPushButton* m_search2CrateButton;

    QString generateQuery() const;
};

#endif // WFASTSEARCH_H
