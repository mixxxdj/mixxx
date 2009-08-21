#include <QtDebug>

#include "library/libraryfeature.h"
#include "library/sidebarmodel.h"

SidebarModel::SidebarModel(QObject* parent)
    : QAbstractItemModel(parent) {
    
}

SidebarModel::~SidebarModel() {

}

void SidebarModel::addLibraryFeature(LibraryFeature* feature) {
    m_sFeatures.push_back(feature);
}

QModelIndex SidebarModel::index(int row, int column,
                                const QModelIndex& parent) const {
    //qDebug("SidebarModel::index row=%d column=%d parent=%8x", row, column, &parent);
    if (parent.isValid()) {
        return createIndex(row, column, m_sFeatures[parent.row()]);
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
            // TODO(rryan) something nicer
            for (int i = 0; i < m_sFeatures.size(); ++i) {
                if (index.internalPointer() == m_sFeatures[i]) {
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
            return m_sFeatures[parent.row()]->numChildren();
        } else {
            // Nested can't currently have children, sorry
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
    //qDebug("SidebarModel::data row=%d column=%d pointer=%8x, role=%d",
    //       index.row(), index.column(), index.internalPointer(), role);
    if (index.isValid()) {
        if (this == index.internalPointer()) {
            if (role == Qt::DisplayRole) {
                return m_sFeatures[index.row()]->title();
            } else if (role == Qt::DecorationRole) {
                return m_sFeatures[index.row()]->getIcon();
            }
        } else {
            for (int i = 0; i < m_sFeatures.size(); ++i) {
                if (m_sFeatures[i] == index.internalPointer()) {
                    if (role == Qt::DisplayRole) {
                        return m_sFeatures[i]->child(index.row());
                    }
                }
            }
            
        }
    }
    return QVariant();
}

void SidebarModel::clicked(const QModelIndex& index) {
    qDebug() << "SidebarModel::clicked() index=" << index;
    if (index.isValid()) {
        if (index.internalPointer() == this) {
            m_sFeatures[index.row()]->activate();
        } else {
            for (int i = 0; i < m_sFeatures.size(); ++i) {
                if (m_sFeatures[i] == index.internalPointer()) {
                    m_sFeatures[i]->activateChild(index.row());
                }
            }
        }
        
    }
}
