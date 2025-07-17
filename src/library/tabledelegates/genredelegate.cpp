#include "library/tabledelegates/genredelegate.h"

#include <QPainter>
#include <QStyle>
#include <QTableView>

// #include "library/basesqltablemodel.h"
#include "library/trackset/tracksettablemodel.h"
#include "moc_genredelegate.cpp"

GenreDelegate::GenreDelegate(QObject* parent)
        : QStyledItemDelegate(parent) {
}

QString GenreDelegate::displayText(const QVariant& value, const QLocale&) const {
    const QString raw = value.toString(); // example: ##2##,##4##
    if (raw.isEmpty()) {
        return QString();
    }

    QStringList ids = raw.split(",", Qt::SkipEmptyParts);
    QStringList names;

    const BaseSqlTableModel* pModel =
            qobject_cast<const BaseSqlTableModel*>(parent());

    if (!pModel) {
        qWarning() << "GenreDelegate: parent is not a BaseSqlTableModel!";
        return raw; // fallback
    }

    for (const QString& id : ids) {
        names << pModel->getDisplayGenreNameForGenreID(id);
    }
    return names.join(", ");
}
