#pragma once

#include <QCompleter>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QStringListModel>

class GenreLineEditor : public QLineEdit {
    Q_OBJECT

  public:
    explicit GenreLineEditor(QWidget* parent = nullptr);

    void setGenreList(const QStringList& genres);
    QStringList genres() const;
    void setInitialGenres(const QStringList& initial);

  protected:
    void keyPressEvent(QKeyEvent* event) override;

  private slots:
    void slotOnTextEdited(const QString& text);
    void slotOnCompletionSelected(const QString& completion);

  private:
    QCompleter* m_completer;
    QStringListModel* m_model;
    QSortFilterProxyModel* m_filterModel;
};
