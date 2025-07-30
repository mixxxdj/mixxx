#include "widget/wgenretaginput.h"

#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>

#include "library/trackcollection.h"
#include "moc_wgenretaginput.cpp"
#include "track/track.h"
#include "util/assert.h"

namespace {
const int kTagSpacing = 4;
} // anonymous namespace

WGenreTagInput::WGenreTagInput(QWidget* parent)
        : QWidget(parent),
          m_pTrackCollection(nullptr),
          m_readOnly(false),
          m_editMode(false),
          m_multiTrackMode(false),
          m_autoSave(true),
          m_pMainLayout(nullptr),
          m_pScrollArea(nullptr),
          m_pTagContainer(nullptr),
          m_pTagLayout(nullptr),
          m_pLineEdit(nullptr),
          m_pAddButton(nullptr),
          m_pDisplayLabel(nullptr),
          m_pCompleter(nullptr),
          m_pCompleterModel(nullptr),
          m_isDragging(false) {
    setupUI();
    setFocusPolicy(Qt::ClickFocus);
    setAttribute(Qt::WA_Hover, true);
    setAcceptDrops(true);
}

void WGenreTagInput::setupUI() {
    m_pMainLayout = new QHBoxLayout(this);
    m_pMainLayout->setContentsMargins(2, 2, 2, 2);
    m_pMainLayout->setSpacing(0);

    // Display label for non-edit mode
    m_pDisplayLabel = new QLabel(this);
    m_pDisplayLabel->setWordWrap(false);
    m_pDisplayLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_pDisplayLabel->setStyleSheet("QLabel { padding: 2px; background: transparent; }");
    m_pDisplayLabel->setVisible(false);
    m_pMainLayout->addWidget(m_pDisplayLabel);

    // Scroll area for tags in edit mode
    m_pScrollArea = new QScrollArea(this);
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pScrollArea->setFixedHeight(26);
    m_pScrollArea->setVisible(true);
    m_pScrollArea->setStyleSheet(
            "QScrollArea { border: 1px solid #555; border-radius: 3px; "
            "background-color: #2a2a2a; }"
            "QScrollBar:horizontal { background-color: #3a3a3a; height: 8px; "
            "border-radius: 4px; }"
            "QScrollBar::handle:horizontal { background-color: #666; "
            "border-radius: 4px; min-width: 20px; }"
            "QScrollBar::handle:horizontal:hover { background-color: #888; }"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal "
            "{ width: 0px; }");

    // Container for genre tags
    m_pTagContainer = new QWidget();
    m_pTagLayout = new QHBoxLayout(m_pTagContainer);
    m_pTagLayout->setContentsMargins(2, 2, 2, 2);
    m_pTagLayout->setSpacing(kTagSpacing);
    m_pTagLayout->addStretch();

    m_pScrollArea->setWidget(m_pTagContainer);
    m_pMainLayout->addWidget(m_pScrollArea);

    // Line edit for adding new genres
    m_pLineEdit = new QLineEdit(this);
    m_pLineEdit->setPlaceholderText(tr("Add genre..."));
    m_pLineEdit->setVisible(true);
    m_pLineEdit->setFixedHeight(26);
    m_pLineEdit->setMinimumWidth(80);
    m_pLineEdit->setMaximumWidth(150);
    m_pLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(m_pLineEdit, &QLineEdit::returnPressed, this, &WGenreTagInput::slotAddGenre);
    connect(m_pLineEdit, &QLineEdit::editingFinished, this, &WGenreTagInput::slotLineEditFinished);
    m_pMainLayout->addWidget(m_pLineEdit);

    // Add button
    m_pAddButton = new QPushButton("+", this);
    m_pAddButton->setMaximumSize(24, 24);
    m_pAddButton->setVisible(true);
    connect(m_pAddButton, &QPushButton::clicked, this, &WGenreTagInput::slotAddGenre);
    m_pMainLayout->addWidget(m_pAddButton);

    m_editMode = true;
    updateGenreTags();
}

void WGenreTagInput::setTrack(TrackPointer pTrack) {
    if (m_pTrack) {
        disconnect(m_pTrack.get(), nullptr, this, nullptr);
    }

    m_pTrack = pTrack;

    if (!m_pTrack) {
        clear();
        return;
    }

    loadGenresFromTrack();
}

void WGenreTagInput::setTrackCollection(TrackCollection* pTrackCollection) {
    qDebug() << "=== setTrackCollection() DEBUG ===";
    qDebug() << "pTrackCollection:" << (pTrackCollection ? "OK" : "NULL");
    qDebug() << "multiTrackMode:" << m_multiTrackMode;

    m_pTrackCollection = pTrackCollection;

    if (m_pTrackCollection && !m_multiTrackMode) {
        setupGenreCompleter();
        qDebug() << "TrackCollection set successfully for single track!";
    } else if (m_multiTrackMode) {
        qDebug() << "Multi-track mode: TrackCollection set but completer not auto-configured";
    }
}

void WGenreTagInput::setupGenreCompleter() {
    if (!m_pTrackCollection)
        return;

    QStringList genreNames = m_pTrackCollection->getGenreDao().getAllGenreNames();

    if (m_pCompleter) {
        m_pCompleter->deleteLater();
    }

    m_pCompleter = new QCompleter(genreNames, this);
    m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_pCompleter->setFilterMode(Qt::MatchContains);
    m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_pCompleter->setMaxVisibleItems(8);

    if (m_pLineEdit) {
        m_pLineEdit->setCompleter(m_pCompleter);
    }

    connect(m_pCompleter,
            QOverload<const QString&>::of(&QCompleter::activated),
            this,
            &WGenreTagInput::slotCompleterActivated);

    qDebug() << "WGenreTagInput::setupGenreCompleter completed with"
             << genreNames.size() << "genres";
}

void WGenreTagInput::loadGenresFromTrack() {
    if (!m_pTrack || !m_pTrackCollection) {
        qDebug() << "WGenreTagInput::loadGenresFromTrack - missing track or collection";
        return;
    }

    QStringList genres = m_pTrackCollection->getGenreDao().getGenresForTrack(m_pTrack->getId());
    qDebug() << "WGenreTagInput::loadGenresFromTrack - loaded genres:" << genres;

    setGenres(genres);
}

void WGenreTagInput::saveGenresToTrack() {
    qDebug() << "=== saveGenresToTrack() DEBUG ===";
    qDebug() << "m_pTrack:" << (m_pTrack ? "OK" : "NULL");
    qDebug() << "m_pTrackCollection:" << (m_pTrackCollection ? "OK" : "NULL");

    if (!m_pTrack || !m_pTrackCollection) {
        qDebug() << "WGenreTagInput::saveGenresToTrack - missing track or collection";
        return;
    }

    qDebug() << "WGenreTagInput::saveGenresToTrack - saving genres:" << m_genres;

    // Save genres to the track in the collection
    // This will update the track's genre information in the database.
    // Ensure that the track ID is valid and the collection is set.
    bool success = m_pTrackCollection->getGenreDao().setGenresForTrack(m_pTrack->getId(), m_genres);

    if (success) {
        emit genresChanged();
        qDebug() << "WGenreTagInput::saveGenresToTrack - genres saved successfully";
    } else {
        qWarning() << "WGenreTagInput::saveGenresToTrack - failed to save genres";
    }
}

QStringList WGenreTagInput::getGenres() const {
    return m_genres;
}

void WGenreTagInput::setGenres(const QStringList& genres) {
    qDebug() << "=== setGenres() Start with:" << genres;

    if (!m_pDisplayLabel) {
        qDebug() << "ERROR: m_pDisplayLabel is null!";
        return;
    }

    qDebug() << "Assegning m_genres...";
    m_genres = genres;

    qDebug() << "Updating display label...";
    if (genres.isEmpty()) {
        m_pDisplayLabel->setText("No genres");
    } else {
        m_pDisplayLabel->setText(genres.join(", "));
    }

    qDebug() << "Calling updateGenreTags()...";
    updateGenreTags();

    qDebug() << "Emitting signal genresChanged()...";
    emit genresChanged();

    qDebug() << "=== setGenres() End ===";
}

void WGenreTagInput::setReadOnly(bool readOnly) {
    m_readOnly = readOnly;

    if (m_readOnly && m_editMode) {
        exitEditMode();
    }

    if (m_pLineEdit) {
        m_pLineEdit->setReadOnly(m_readOnly);
    }

    if (m_pAddButton) {
        m_pAddButton->setVisible(!m_readOnly);
    }

    update();
}

bool WGenreTagInput::isReadOnly() const {
    return m_readOnly;
}

void WGenreTagInput::setCompleterModel(QStringListModel* model) {
    m_pCompleterModel = model;
    if (m_pCompleterModel && m_pLineEdit) {
        if (!m_pCompleter) {
            m_pCompleter = new QCompleter(this);
            m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
            m_pCompleter->setFilterMode(Qt::MatchStartsWith);
            m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
            m_pCompleter->setMaxVisibleItems(8);
            connect(m_pCompleter,
                    QOverload<const QString&>::of(&QCompleter::activated),
                    this,
                    &WGenreTagInput::slotCompleterActivated);
        }
        m_pCompleter->setModel(m_pCompleterModel);
        m_pLineEdit->setCompleter(m_pCompleter);
    }
}

void WGenreTagInput::addGenre(const QString& genre) {
    qDebug() << "=== addGenre() Start with:" << genre;
    qDebug() << "autoSave:" << m_autoSave << "multiTrackMode:" << m_multiTrackMode;

    const QString trimmed = genre.trimmed();
    if (trimmed.isEmpty() || m_genres.contains(trimmed, Qt::CaseInsensitive)) {
        qDebug() << "Genre empty or duplicated, exit..";
        return;
    }

    qDebug() << "Creating genres list";
    QStringList newGenres = m_genres;
    newGenres.append(trimmed);

    qDebug() << "New List:" << newGenres;
    setGenres(newGenres);

    if (m_autoSave && !m_multiTrackMode) {
        qDebug() << "Auto-saving after genre addition";
        saveGenresToTrack();
    } else {
        qDebug() << "Auto-save disabled - not saving to track";
    }

    qDebug() << "=== addGenre() End ===";
}

void WGenreTagInput::removeGenre(const QString& genre) {
    qDebug() << "WGenreTagInput::removeGenre called with:" << genre;
    qDebug() << "autoSave:" << m_autoSave << "multiTrackMode:" << m_multiTrackMode;

    QStringList newGenres = m_genres;
    if (newGenres.removeOne(genre)) {
        qDebug() << "Genre removed successfully. New list:" << newGenres;
        setGenres(newGenres);

        if (m_autoSave && !m_multiTrackMode) {
            qDebug() << "Auto-saving after genre removal";
            saveGenresToTrack();
        } else {
            qDebug() << "Auto-save disabled - not saving to track";
        }
    } else {
        qDebug() << "Warning: Genre not found in list:" << genre;
    }
}

void WGenreTagInput::clear() {
    setGenres(QStringList());
}

void WGenreTagInput::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}

void WGenreTagInput::mouseDoubleClickEvent(QMouseEvent* event) {
    QWidget::mouseDoubleClickEvent(event);
}

void WGenreTagInput::keyPressEvent(QKeyEvent* event) {
    QWidget::keyPressEvent(event);
}

void WGenreTagInput::focusInEvent(QFocusEvent* event) {
    QWidget::focusInEvent(event);
    update();
}

void WGenreTagInput::focusOutEvent(QFocusEvent* event) {
    QWidget::focusOutEvent(event);
    update();
}

void WGenreTagInput::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
        QWidget* clickedWidget = childAt(event->pos());
        while (clickedWidget && clickedWidget != this) {
            QString genre = clickedWidget->property("genre").toString();
            if (!genre.isEmpty()) {
                m_draggedGenre = genre;
                break;
            }
            clickedWidget = clickedWidget->parentWidget();
        }
    }
    QWidget::mousePressEvent(event);
}

void WGenreTagInput::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton) || m_draggedGenre.isEmpty() ||
            (event->pos() - m_dragStartPosition).manhattanLength() <
                    QApplication::startDragDistance()) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    mimeData->setText("mixxx-genre:" + m_draggedGenre);
    drag->setMimeData(mimeData);

    QLabel tempLabel(m_draggedGenre);
    tempLabel.setStyleSheet(
            "QLabel { background-color: #3a3a3a; color: #ddd; padding: 4px; "
            "border-radius: 3px; }");
    tempLabel.adjustSize();
    QPixmap pixmap = tempLabel.grab();
    drag->setPixmap(pixmap);

    m_isDragging = true;
    drag->exec(Qt::MoveAction);
    m_isDragging = false;
    m_draggedGenre.clear();
}

void WGenreTagInput::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasText() && event->mimeData()->text().startsWith("mixxx-genre:")) {
        event->acceptProposedAction();
    }
}

void WGenreTagInput::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasText() && event->mimeData()->text().startsWith("mixxx-genre:")) {
        event->acceptProposedAction();
    }
}

void WGenreTagInput::dropEvent(QDropEvent* event) {
    QString mimeText = event->mimeData()->text();
    if (!mimeText.startsWith("mixxx-genre:")) {
        return;
    }

    QString droppedGenre = mimeText.mid(12);
    QPoint dropPos = event->position().toPoint();
    int targetIndex = 0;

    if (m_pScrollArea && m_pScrollArea->geometry().contains(dropPos)) {
        QPoint containerPos = m_pTagContainer->mapFromParent(m_pScrollArea->mapFromParent(dropPos));
        for (int i = 0; i < m_pTagLayout->count() - 1; ++i) {
            if (QLayoutItem* item = m_pTagLayout->itemAt(i)) {
                if (item->widget()) {
                    if (containerPos.x() < item->widget()->geometry().center().x()) {
                        targetIndex = i;
                        break;
                    }
                    targetIndex = i + 1;
                }
            }
        }
    }

    QStringList newGenres = m_genres;
    int currentIndex = newGenres.indexOf(droppedGenre);
    if (currentIndex != -1 && currentIndex != targetIndex) {
        newGenres.removeAt(currentIndex);
        if (targetIndex > currentIndex) {
            targetIndex--;
        }
        if (targetIndex >= newGenres.size()) {
            newGenres.append(droppedGenre);
        } else {
            newGenres.insert(targetIndex, droppedGenre);
        }
        setGenres(newGenres);

        if (m_autoSave && !m_multiTrackMode) {
            saveGenresToTrack();
        }

        emit genresChanged();
    }
    event->acceptProposedAction();
}

void WGenreTagInput::debugState() const {
    qDebug() << "=== WGenreTagInput Debug State ===";
    qDebug() << "Current genres:" << m_genres;
    qDebug() << "Multi-track mode:" << m_multiTrackMode;
    qDebug() << "Auto-save enabled:" << m_autoSave;
    qDebug() << "Read-only:" << m_readOnly;
    qDebug() << "Edit mode:" << m_editMode;
    qDebug() << "Has TrackCollection:" << (m_pTrackCollection ? "YES" : "NO");
    qDebug() << "Has Track:" << (m_pTrack ? "YES" : "NO");
    qDebug() << "=== End Debug State ===";
}

void WGenreTagInput::enterEditMode() {
    if (m_editMode || m_readOnly) {
        return;
    }

    m_editMode = true;
    m_pDisplayLabel->setVisible(false);
    m_pScrollArea->setVisible(true);
    m_pLineEdit->setVisible(true);
    m_pAddButton->setVisible(true);
    updateGenreTags();
    m_pLineEdit->setFocus();
}

void WGenreTagInput::exitEditMode() {
    if (!m_editMode) {
        return;
    }

    m_editMode = false;
    m_pScrollArea->setVisible(false);
    m_pLineEdit->setVisible(false);
    m_pAddButton->setVisible(false);
    m_pDisplayLabel->setVisible(true);
    updateGenreTags();
    emit editingFinished();
}

void WGenreTagInput::updateGenreTags() {
    qDebug() << "=== updateGenreTags() Start ===";

    if (!m_pTagLayout) {
        qDebug() << "ERRORE: m_pTagLayout is null!";
        return;
    }

    if (m_editMode) {
        qDebug() << "Edit Mode: cleaning existing layout";

        // Clean existing tags
        while (QLayoutItem* item = m_pTagLayout->takeAt(0)) {
            qDebug() << "Removing Item form layout...";
            if (item->widget()) {
                qDebug() << "Deleting widget...";
                item->widget()->deleteLater();
            }
            delete item;
        }

        m_pTagLayout->addStretch();

        qDebug() << "Adding new tag for" << m_genres.size() << "genres...";

        // Add new tags for each genre
        for (const QString& genre : m_genres) {
            qDebug() << "Creating genre tag:" << genre;
            QWidget* tagWidget = createGenreTag(genre);
            if (tagWidget) {
                qDebug() << "Tag created, adding to layout";
                m_pTagLayout->insertWidget(m_pTagLayout->count() - 1, tagWidget);
                qDebug() << "Tag added to layout";
            } else {
                qDebug() << "ERROR: createGenreTag returns nullptr!";
            }
        }
    } else {
        qDebug() << "display mode: updating label...";
        if (m_pDisplayLabel) {
            m_pDisplayLabel->setText(formatGenresForDisplay());
        }
    }

    qDebug() << "=== updateGenreTags() End ===";
}

QString WGenreTagInput::formatGenresForDisplay() const {
    if (m_genres.isEmpty()) {
        return tr("No genres");
    }
    return m_genres.join(", ");
}

QWidget* WGenreTagInput::createGenreTag(const QString& genre) {
    qDebug() << "=== createGenreTag() Start for:" << genre;

    QWidget* tagWidget = new QWidget();
    if (!tagWidget) {
        qDebug() << "ERROR: Impossible to create tagWidget";
        return nullptr;
    }

    tagWidget->setStyleSheet(
            "QWidget { background-color: #3a3a3a; border: 1px solid #555; "
            "border-radius: 3px; padding: 2px 6px; }"
            "QWidget:hover { background-color: #4a4a4a; }");
    tagWidget->setFixedHeight(22);
    tagWidget->setProperty("genre", genre);

    QHBoxLayout* layout = new QHBoxLayout(tagWidget);
    if (!layout) {
        qDebug() << "ERROR: Impossible to create layout!";
        tagWidget->deleteLater();
        return nullptr;
    }

    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(4);

    QLabel* label = new QLabel(genre, tagWidget);
    if (!label) {
        qDebug() << "ERROR: Impossible to create label!";
        tagWidget->deleteLater();
        return nullptr;
    }

    label->setStyleSheet("QLabel { background: transparent; border: none; color: #ddd; }");
    layout->addWidget(label);

    QPushButton* removeButton = new QPushButton("×", tagWidget);
    if (!removeButton) {
        qDebug() << "ERROR: Impossible to create removeButton!";
        tagWidget->deleteLater();
        return nullptr;
    }

    removeButton->setMaximumSize(16, 16);
    removeButton->setStyleSheet(
            "QPushButton { background: transparent; border: none; color: #ccc; font-weight: bold; }"
            "QPushButton:hover { color: #fff; background-color: #666; border-radius: 8px; }");

    connect(removeButton, &QPushButton::clicked, this, [this, genre]() {
        qDebug() << "Removing genre:" << genre;
        removeGenre(genre);
    });

    layout->addWidget(removeButton);

    qDebug() << "=== createGenreTag() End - tag created successfully ===";
    return tagWidget;
}

void WGenreTagInput::slotTrackGenresChanged(const QStringList& genres) {
    setGenres(genres);
}

void WGenreTagInput::slotAddGenre() {
    qDebug() << "=== slotAddGenre() Starting ===";

    if (!m_pLineEdit) {
        qDebug() << "CRITICAL ERROR: m_pLineEdit is null!";
        return;
    }

    QString text = m_pLineEdit->text().trimmed();
    qDebug() << "Text from LineEdit:" << text;

    if (!text.isEmpty()) {
        qDebug() << "Calling addGenre()...";
        addGenre(text);
        qDebug() << "addGenre() completed, cleaning LineEdit...";
        m_pLineEdit->clear();
        qDebug() << "LineEdit cleared";

        if (m_autoSave && !m_multiTrackMode) {
            qDebug() << "Auto-saving after slot addition";
            saveGenresToTrack();
        }
    }

    qDebug() << "=== slotAddGenre() End ===";
}

void WGenreTagInput::slotRemoveGenre() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        if (QWidget* tagWidget = button->parentWidget()) {
            if (QLabel* label = tagWidget->findChild<QLabel*>()) {
                removeGenre(label->text());
            }
        }
    }
}

void WGenreTagInput::slotLineEditFinished() {
    QString text = m_pLineEdit->text().trimmed();
    if (!text.isEmpty()) {
        addGenre(text);
        m_pLineEdit->clear();

        if (m_autoSave && !m_multiTrackMode) {
            saveGenresToTrack();
        }
    }
}

void WGenreTagInput::slotCompleterActivated(const QString& text) {
    addGenre(text);
    m_pLineEdit->clear();

    if (m_autoSave && !m_multiTrackMode) {
        saveGenresToTrack();
    }
}

void WGenreTagInput::setMultiTrackMode(bool multiTrack) {
    qDebug() << "WGenreTagInput::setMultiTrackMode:" << multiTrack;
    m_multiTrackMode = multiTrack;

    if (m_multiTrackMode) {
        setAutoSave(false);
    }
}

bool WGenreTagInput::isMultiTrackMode() const {
    return m_multiTrackMode;
}

void WGenreTagInput::setAutoSave(bool autoSave) {
    qDebug() << "WGenreTagInput::setAutoSave:" << autoSave;
    m_autoSave = autoSave;
}

bool WGenreTagInput::isAutoSave() const {
    return m_autoSave;
}

void WGenreTagInput::setGenreCompleter(const QStringList& genres) {
    qDebug() << "WGenreTagInput::setGenreCompleter with" << genres.size() << "genres";

    if (m_pCompleter) {
        m_pCompleter->deleteLater();
    }

    m_pCompleter = new QCompleter(genres, this);
    m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_pCompleter->setFilterMode(Qt::MatchContains);
    m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_pCompleter->setMaxVisibleItems(8);

    if (m_pLineEdit) {
        m_pLineEdit->setCompleter(m_pCompleter);
    }

    connect(m_pCompleter,
            QOverload<const QString&>::of(&QCompleter::activated),
            this,
            &WGenreTagInput::slotCompleterActivated);

    qDebug() << "Genre completer configured successfully";
}
