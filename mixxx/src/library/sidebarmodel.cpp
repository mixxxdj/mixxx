#include <QtDebug>
#include <QUrl>
#include <QApplication>

#include "library/libraryfeature.h"
#include "library/sidebarmodel.h"

const int AUTO_EXPAND_TIME = 500;

SidebarModel::SidebarModel(QObject* parent)
    : m_iDefaultSelectedIndex(0),
      QAbstractItemModel(parent) {
}

SidebarModel::~SidebarModel() {

}

void SidebarModel::addLibraryFeature(LibraryFeature* feature) {
    m_sFeatures.push_back(feature);
    connect(feature, SIGNAL(featureUpdated()), this, SLOT(refreshData()));
    QAbstractItemModel* model = feature->getChildModel();

    connect(model, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)),
            this, SLOT(slotDataChanged(const QModelIndex&,const QModelIndex&)));
    connect(model, SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int, int)),
            this, SLOT(slotRowsAboutToBeInserted(const QModelIndex&, int, int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(slotRowsAboutToBeRemoved(const QModelIndex&, int, int)));
    connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(slotRowsInserted(const QModelIndex&, int, int)));
    connect(model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));

    m_autoExpandTimer.setInterval(AUTO_EXPAND_TIME);
    m_autoExpandTimer.stop();

    connect( &m_autoExpandTimer, SIGNAL(timeout()), this, SLOT(slotAutoExpandTimerTimeout()) );
}

QModelIndex SidebarModel::getDefaultSelection() {
    if (m_sFeatures.size() == 0)
        return QModelIndex();
    return createIndex(m_iDefaultSelectedIndex, 0, (void*)this);
}

void SidebarModel::setDefaultSelection(unsigned int index)
{
    m_iDefaultSelectedIndex = index;
}

void SidebarModel::activateDefaultSelection() {
    if (m_sFeatures.size() > 0) {
        m_sFeatures[m_iDefaultSelectedIndex]->activate();
    }
}

void SidebarModel::refreshData()
{
    //Reset all the model indices and refresh all the data.
    //TODO: Could do something nicer when a feature's children change,
    //      but the features know nothing about their model indices,
    //      so they can't do stuff like beginInsertRow() to help the
    //      model manage the indices.
    //reset();
}

QModelIndex SidebarModel::index(int row, int column,
                                const QModelIndex& parent) const {
    // qDebug() << "SidebarModel::index row=" << row
    //          << "column=" << column << "parent=" << parent;
    if (parent.isValid()) {
        const QAbstractItemModel* childModel;
        if (parent.internalPointer() == this) {
            childModel = m_sFeatures[parent.row()]->getChildModel();
        } else {
            childModel = (QAbstractItemModel*)parent.internalPointer();
        }
        QModelIndex childIndex = childModel->index(row, column);

        if (childIndex.isValid()) {
            return createIndex(childIndex.row(), childIndex.column(),
                               (void*)childModel);
        } else {
            return QModelIndex();
        }
    }
    return createIndex(row, column, (void*)this);
}

QModelIndex SidebarModel::parent(const QModelIndex& index) const {
    //qDebug() << "SidebarModel::parent index=" << index;
    if (index.isValid()) {
        if (index.internalPointer() == this) {
            // Top level, no parent
            return QModelIndex();
        } else {
            const QAbstractItemModel* childModel = (QAbstractItemModel*)index.internalPointer();

            for (int i = 0; i < m_sFeatures.size(); ++i) {
                if (childModel == m_sFeatures[i]->getChildModel()) {
                    return createIndex(i, 0, (void*)this);
                }
            }
        }
    }
    return QModelIndex();
}

int SidebarModel::rowCount(const QModelIndex& parent) const {
    //qDebug() << "SidebarModel::rowCount parent=" << parent;
    if (parent.isValid()) {
        if (parent.internalPointer() == this) {
            return m_sFeatures[parent.row()]->getChildModel()->rowCount();
        } else {
            // We don't support feature models any deeper than 1 level.
            return 0;
        }
    }
    return m_sFeatures.size();
}

int SidebarModel::columnCount(const QModelIndex& parent) const {
    //qDebug() << "SidebarModel::columnCount parent=" << parent;
    // TODO(rryan) will we ever have columns? I don't think so.
    return 1;
}

QVariant SidebarModel::data(const QModelIndex& index, int role) const {
    // qDebug("SidebarModel::data row=%d column=%d pointer=%8x, role=%d",
    //        index.row(), index.column(), index.internalPointer(), role);
    if (index.isValid()) {
        if (this == index.internalPointer()) {
            if (role == Qt::DisplayRole) {
                return m_sFeatures[index.row()]->title();
            } else if (role == Qt::DecorationRole) {
                return m_sFeatures[index.row()]->getIcon();
            }
        } else {
            QAbstractItemModel* childModel = (QAbstractItemModel*)index.internalPointer();
            return childModel->data(childModel->index(index.row(), index.column()), role);
        }
    }
    return QVariant();
}

void SidebarModel::clicked(const QModelIndex& index) {
    qDebug() << "SidebarModel::clicked() index=" << index;

    if (QApplication::mouseButtons() != Qt::LeftButton) {
        return;
    }

    if (index.isValid()) {
        if (index.internalPointer() == this) {
            m_sFeatures[index.row()]->activate();
        } else {
            QAbstractItemModel* childModel = (QAbstractItemModel*)index.internalPointer();
            QModelIndex childIndex = childModel->index(index.row(), index.column());
            for (int i = 0; i < m_sFeatures.size(); ++i) {
                if (m_sFeatures[i]->getChildModel() == childModel) {
                    m_sFeatures[i]->activateChild(childIndex);
                }
            }
        }
    }
}

void SidebarModel::rightClicked(const QPoint& globalPos, const QModelIndex& index) {
    qDebug() << "SidebarModel::rightClicked() index=" << index;
    if (index.isValid()) {
        if (index.internalPointer() == this) {
            m_sFeatures[index.row()]->activate();
            m_sFeatures[index.row()]->onRightClick(globalPos);
        } else {
            QAbstractItemModel* childModel = (QAbstractItemModel*)index.internalPointer();
            QModelIndex childIndex = childModel->index(index.row(), index.column());
            for (int i = 0; i < m_sFeatures.size(); ++i) {
                if (m_sFeatures[i]->getChildModel() == childModel) {
                    m_sFeatures[i]->activateChild(childIndex);
                    m_sFeatures[i]->onRightClickChild(globalPos, childIndex);
                }
            }
        }
    }
}

bool SidebarModel::dropAccept(const QModelIndex& index, QUrl url)
{
    //qDebug() << "SidebarModel::dropAccept() index=" << index << url;
    if (index.isValid()) {
        if (index.internalPointer() == this) {
            return m_sFeatures[index.row()]->dropAccept(url);
        } else {
            QAbstractItemModel* childModel = (QAbstractItemModel*)index.internalPointer();
            QModelIndex childIndex = childModel->index(index.row(), index.column());
            for (int i = 0; i < m_sFeatures.size(); ++i) {
                if (m_sFeatures[i]->getChildModel() == childModel) {
                    return m_sFeatures[i]->dropAcceptChild(childIndex, url);
                }
            }
        }
    }

    return false;
}

bool SidebarModel::dragMoveAccept(const QModelIndex& index, QUrl url)
{
    //qDebug() << "SidebarModel::dragMoveAccept() index=" << index << url;
    if (index.isValid()) {
        if (index.internalPointer() == this) {
            m_hoveredIndex = index;
            m_autoExpandTimer.start();
            return m_sFeatures[index.row()]->dragMoveAccept(url);
        } else {
            m_autoExpandTimer.stop();
            QAbstractItemModel* childModel = (QAbstractItemModel*)index.internalPointer();
            QModelIndex childIndex = childModel->index(index.row(), index.column());
            for (int i = 0; i < m_sFeatures.size(); ++i) {
                if (m_sFeatures[i]->getChildModel() == childModel) {
                    return m_sFeatures[i]->dragMoveAcceptChild(childIndex, url);
                }
            }
        }
    }
    return false;
}

QModelIndex SidebarModel::translateSourceIndex(const QModelIndex& index) {
    QModelIndex translatedIndex;
    const QAbstractItemModel* model = (QAbstractItemModel*)sender();
    Q_ASSERT(model);
    if (index.isValid()) {
        translatedIndex = createIndex(index.row(), index.column(),
                                      (void*)model);
    } else {
        for (int i = 0; i < m_sFeatures.size(); ++i) {
            if (m_sFeatures[i]->getChildModel() == model) {
                translatedIndex = createIndex(i, 0, (void*)this);
            }
        }
    }
    return translatedIndex;
}

void SidebarModel::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
    //qDebug() << "slotDataChanged topLeft:" << topLeft << "bottomRight:" << bottomRight;
}

void SidebarModel::slotRowsAboutToBeInserted(const QModelIndex& parent, int start, int end) {
    //qDebug() << "slotRowsABoutToBeInserted" << parent << start << end;
    QModelIndex newParent = translateSourceIndex(parent);
    beginInsertRows(newParent, start, end);
}

void SidebarModel::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end) {
    //qDebug() << "slotRowsABoutToBeRemoved" << parent << start << end;
    QModelIndex newParent = translateSourceIndex(parent);
    beginRemoveRows(newParent, start, end);
}

void SidebarModel::slotRowsInserted(const QModelIndex& parent, int start, int end) {
    //qDebug() << "slotRowsInserted" << parent << start << end;
    //QModelIndex newParent = translateSourceIndex(parent);
    endInsertRows();
}

void SidebarModel::slotRowsRemoved(const QModelIndex& parent, int start, int end) {
    //qDebug() << "slotRowsRemoved" << parent << start << end;
    //QModelIndex newParent = translateSourceIndex(parent);
    endRemoveRows();
}

void SidebarModel::slotAutoExpandTimerTimeout()
{
    emit expandIndex(m_hoveredIndex);
}
