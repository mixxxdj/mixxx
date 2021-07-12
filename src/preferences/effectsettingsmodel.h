#pragma once

#include <QAbstractTableModel>
#include <QAbstractItemDelegate>
#include <QMap>
#include <QVariant>

#include "effects/effectmanifest.h"
#include "effects/effectsmanager.h"

struct EffectProfile {
    EffectManifestPointer pManifest;
    bool bIsVisible;

    EffectProfile(EffectManifestPointer _pManifest, bool _bIsVisible) {
        pManifest = _pManifest;
        bIsVisible = _bIsVisible;
    }
};
typedef QSharedPointer<EffectProfile> EffectProfilePtr;

class EffectSettingsModel : public QAbstractTableModel {
  Q_OBJECT
  public:
    EffectSettingsModel();
    ~EffectSettingsModel();

    void resetFromEffectManager(EffectsManager* pEffectsManager);

    bool addProfileToModel(EffectProfilePtr profile);
    void deleteProfileFromModel(EffectProfilePtr profile);
    QList<EffectProfilePtr> profiles() {
        return m_profiles;
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value,
            int role = Qt::EditRole);
    QAbstractItemDelegate* delegateForColumn(const int i, QObject* parent);

    bool isEmpty() const;

  private:
    QList<EffectProfilePtr> m_profiles;
};
