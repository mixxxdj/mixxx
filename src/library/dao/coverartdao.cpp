#include <QtDebug>
#include <QThread>

#include "library/dao/coverartdao.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database)
    : m_database(database) {
}

CoverArtDAO::~CoverArtDAO() {
}

void CoverArtDAO::initialize() {
    qDebug() << "CoverArtDAO::initialize" << QThread::currentThread() << m_database.connectionName();
}
