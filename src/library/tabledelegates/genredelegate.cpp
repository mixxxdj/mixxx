#include "library/tabledelegates/genredelegate.h"

#include <QPainter>
#include <QStyle>
#include <QTableView>
#include <utility>

#include "library/dao/genredao.h"
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
        qDebug() << "[GenreDelegate] -> name: " << name;
        if (!name.isEmpty()) {
            names << name;
            continue;
        }
    }

    return names.join("; ");
}
