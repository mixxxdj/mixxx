#ifndef ABSTRACTMODELROLES_H
#define ABSTRACTMODELROLES_H

#include <QAbstractItemModel>

enum AbstractRole {
    RoleDataPath = Qt::UserRole,
    RoleBold,
    RoleDivider,
    RoleQuery,
    RoleBreadCrumb,
    RoleSettings,
    RoleGroupingLetter
};

#endif // ABSTRACTMODELROLES_H
