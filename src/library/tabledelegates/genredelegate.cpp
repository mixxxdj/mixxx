#include "library/tabledelegates/genredelegate.h"

#include <QCompleter>
#include <QHelpEvent>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>
#include <QTableView>
#include <QToolTip>
#include <utility>

#include "library/dao/genredao.h"
#include "library/tabledelegates/genrelineeditor.h"
#include "library/trackset/tracksettablemodel.h"
#include "moc_genredelegate.cpp"

GenreDelegate::GenreDelegate(GenreDao* pGenreDao, QObject* parent)
        : QStyledItemDelegate(parent),
          m_pGenreDao(pGenreDao) {
    Q_ASSERT(m_pGenreDao);
}

QString GenreDelegate::displayText(const QVariant& value, const QLocale&) const {
    const QString raw = value.toString(); // example: ##2##,##4##
    if (raw.isEmpty()) {
        return QString();
    }

    QStringList ids = raw.split(";", Qt::SkipEmptyParts);
    QStringList names;

    const BaseSqlTableModel* pModel =
            qobject_cast<const BaseSqlTableModel*>(parent());

    if (!pModel) {
        qWarning() << "GenreDelegate: parent is not a BaseSqlTableModel!";
        return raw; // fallback
    }

    for (const QString& id : std::as_const(ids)) {
        QString name = m_pGenreDao->getDisplayGenreNameForGenreID(id);
        // qDebug() << "[GenreDelegate] -> name: " << name;
        if (!name.isEmpty()) {
            names << name;
            continue;
        }
    }

    return names.join("; ");
}

bool GenreDelegate::helpEvent(QHelpEvent* event,
        QAbstractItemView* view,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) {
    if (!event || !view) {
        return false;
    }

    const QVariant rawValue = index.data(Qt::DisplayRole);
    const QString raw = rawValue.toString();
    if (raw.isEmpty()) {
        return false;
    }

    QStringList ids = raw.split(";", Qt::SkipEmptyParts);
    QStringList names;

    for (const QString& id : std::as_const(ids)) {
        QString name = m_pGenreDao->getDisplayGenreNameForGenreID(id);
        if (!name.isEmpty()) {
            names << name;
        }
    }

    QString tooltip = names.join("\n");
    if (!tooltip.isEmpty()) {
        QFont tooltipFont = QToolTip::font();
        tooltipFont.setPointSize(20);
        QToolTip::setFont(tooltipFont);

        QToolTip::showText(event->globalPos(), tooltip, view);
        return true;
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QWidget* GenreDelegate::createEditor(QWidget* parent,
        const QStyleOptionViewItem&,
        const QModelIndex&) const {
    auto* editor = new GenreLineEditor(parent);
    editor->setGenreList(m_pGenreDao->getAllGenres().values());
    // qDebug() << "[GenreDelegate] -> vreateEditor calls (after values)";
    return editor;
}

void GenreDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto* genreEditor = qobject_cast<GenreLineEditor*>(editor);
    if (!genreEditor) {
        return;
    }

    QString raw = index.model()->data(index, Qt::EditRole).toString();
    QStringList ids = raw.split(";", Qt::SkipEmptyParts);
    QStringList names;
    // qDebug() << "[GenreDelegate] -> names: " << names;
    for (const QString& id : std::as_const(ids)) {
        QString name = m_pGenreDao->getDisplayGenreNameForGenreID(id);
        if (!name.isEmpty()) {
            names << name;
        }
    }
    genreEditor->setInitialGenres(names);
}

void GenreDelegate::setModelData(QWidget* editor,
        QAbstractItemModel* model,
        const QModelIndex& index) const {
    auto* genreEditor = qobject_cast<GenreLineEditor*>(editor);
    if (!genreEditor) {
        return;
    }

    QStringList names = genreEditor->genres();
    // qDebug() << "[GenreDelegate] -> names: " << names;
    QString encoded = m_pGenreDao->getIdsForGenreNames(names.join("; "));
    model->setData(index, encoded, Qt::EditRole);

    QList<GenreId> genreIds = m_pGenreDao->getGenreIdsFromIdString(encoded);

    TrackId trackId;
    QVariant trackIdData = model->data(
            index.siblingAtColumn(ColumnCache::COLUMN_LIBRARYTABLE_ID),
            Qt::EditRole);
    qDebug() << "trackIdData; " << trackIdData;

    if (trackIdData.isValid()) {
        trackId = TrackId(trackIdData);
        m_pGenreDao->updateGenreTracksForTrack(trackId, genreIds);
    } else {
        qWarning() << "[GenreDelegate] Could not retrieve TrackId for updateGenreTracksForTrack";
        return;
    }
}
