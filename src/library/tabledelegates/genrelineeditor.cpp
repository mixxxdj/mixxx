#include "library/tabledelegates/genrelineeditor.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStringList>

#include "moc_genrelineeditor.cpp"

GenreLineEditor::GenreLineEditor(QWidget* parent)
        : QLineEdit(parent),
          m_completer(new QCompleter(this)),
          m_model(new QStringListModel(this)),
          m_filterModel(new QSortFilterProxyModel(this)) {
    m_filterModel->setSourceModel(m_model);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel->setFilterRole(Qt::DisplayRole);

    m_completer->setModel(m_filterModel);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_completer->setWidget(this);

    connect(m_completer,
            QOverload<const QString&>::of(&QCompleter::activated),
            this,
            &GenreLineEditor::slotOnCompletionSelected);
    connect(this,
            &QLineEdit::textEdited,
            this,
            &GenreLineEditor::slotOnTextEdited);
}

void GenreLineEditor::slotOnCompletionSelected(const QString& completion) {
    QString current = text();
    int cursorPos = cursorPosition();
    int lastSemi = QStringView(current).left(cursorPos).lastIndexOf(';');

    QString before;
    QString after;

    if (lastSemi != -1) {
        before = current.left(lastSemi + 1);
        after = current.mid(cursorPos);
    }

    QString newText = before + " " + completion + "; " + after;
    setText(newText.trimmed());
    QString inserted = before + " " + completion + "; ";
    setText(inserted + after);
    setCursorPosition(inserted.length());
}

void GenreLineEditor::setGenreList(const QStringList& genres) {
    m_model->setStringList(genres);
}

void GenreLineEditor::setInitialGenres(const QStringList& initial) {
    setText(initial.join("; "));
}

QStringList GenreLineEditor::genres() const {
    QStringList rawList = text().split(';', Qt::SkipEmptyParts);
    QSet<QString> uniqueSet;
    QStringList cleanList;

    for (const QString& genre : std::as_const(rawList)) {
        QString trimmedGenre = genre.trimmed();
        if (!trimmedGenre.isEmpty()) {
            QString lower = trimmedGenre.toLower();
            if (!uniqueSet.contains(lower)) {
                uniqueSet.insert(lower);
                cleanList << trimmedGenre;
            }
        }
    }
    return cleanList;
}

void GenreLineEditor::keyPressEvent(QKeyEvent* event) {
    QLineEdit::keyPressEvent(event);

    if (!m_completer) {
        return;
    }

    QString currentText = text().left(cursorPosition());
    int lastSemicolon = currentText.lastIndexOf(';');
    QString prefix = (lastSemicolon != -1) ? currentText.mid(lastSemicolon + 1).trimmed()
                                           : currentText.trimmed();

    if (!prefix.isEmpty()) {
        m_completer->setCompletionPrefix(prefix);
        m_completer->complete();
    } else {
        m_completer->popup()->hide();
    }
}

void GenreLineEditor::slotOnTextEdited(const QString& text) {
    QString currentText = text.left(cursorPosition());
    int lastSemicolon = currentText.lastIndexOf(';');
    QString prefix = (lastSemicolon != -1)
            ? currentText.mid(lastSemicolon + 1).trimmed()
            : currentText.trimmed();

    if (!prefix.isEmpty()) {
        m_filterModel->setFilterFixedString(prefix);
        m_completer->complete();
    } else {
        m_completer->popup()->hide();
    }
}
