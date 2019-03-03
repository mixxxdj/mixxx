#ifndef MIXXX_CRATETABLEMODEL_H
#define MIXXX_CRATETABLEMODEL_H


#include "library/basesqltablemodel.h"

#include "library/crate/crateid.h"
#include "library/dao/settingsdao.h"


class CrateTableModel : public BaseSqlTableModel {
    Q_OBJECT

  public:
    CrateTableModel(QObject* parent, TrackCollection* pTrackCollection);
    ~CrateTableModel() final;

    void selectCrate(
        CrateId crateId = CrateId());
    CrateId selectedCrate() const {
        return m_selectedCrate;
    }

    bool addTrack(const QModelIndex &index, QString location);

    // From TrackModel
    bool isColumnInternal(int column) final;
    void removeTracks(const QModelIndexList& indices) final;
    // Returns the number of unsuccessful track additions
    int addTracks(const QModelIndex& index, const QList<QString>& locations) final;
    CapabilitiesFlags getCapabilities() const final;

    QString getModelSetting(QString name) override {
        SettingsDAO settings(m_db);
        QString key = m_settingsNamespace + "." + name;
        if (!m_selectedCrate.isValid()) {
          return settings.getValue(key);
        }

        // The default value is the top-level crate setting, which gets updated
        // every time a change is made, so that newly-seen crates will look like
        // the last-edited crate.
        QString defaultVal = settings.getValue(key);
        key = m_settingsNamespace + "." + m_selectedCrate.toString() + "." + name;
        return settings.getValue(key, defaultVal);
    }

    bool setModelSetting(QString name, QVariant value) override {
        SettingsDAO settings(m_db);
        // Also set the top-level crate setting so the next default initialization
        // will use the most recent value created.
        QString key = m_settingsNamespace + "." + name;
        if (!settings.setValue(key, value)) {
          return false;
        }

        if (!m_selectedCrate.isValid()) {
          return true;
        }
     
        key = m_settingsNamespace + "." + m_selectedCrate.toString() + "." + name;
        return settings.setValue(key, value);
    }

  private:
    CrateId m_selectedCrate;
};


#endif // MIXXX_CRATETABLEMODEL_H
