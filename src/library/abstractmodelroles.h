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
    RoleFirstLetter
};

#endif // ABSTRACTMODELROLES_H
