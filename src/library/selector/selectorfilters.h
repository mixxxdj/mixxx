#ifndef SELECTORFILTERS_H
#define SELECTORFILTERS_H

#include "configobject.h"
#include "trackinfoobject.h"

class SelectorFilters : public QObject {
    Q_OBJECT
  public:
    SelectorFilters(QObject* pParent,
                    ConfigObject<ConfigValue>* pConfig);
    virtual ~SelectorFilters();

  public slots:
    void loadStoredFilterSettings();
    void clear();
    void setGenreFilter(bool value);
    void setBpmFilter(bool value, int range);
    void setKeyFilter(bool value);
    void setKey4thFilter(bool value);
    void setKey5thFilter(bool value);
    void setKeyRelativeFilter(bool value);

    // get a filter string in relation to the info of pTrack
    QString getFilterString(TrackPointer pTrack);

  private:
    ConfigObject<ConfigValue>* m_pConfig;

    QList<mixxx::track::io::key::ChromaticKey> getHarmonicKeys(
            mixxx::track::io::key::ChromaticKey key);

    bool m_bFilterGenre;
    bool m_bFilterBpm;
    int m_iFilterBpmRange;
    bool m_bFilterKey;
    bool m_bFilterKey4th;
    bool m_bFilterKey5th;
    bool m_bFilterKeyRelative;
};

#endif // SELECTORFILTERS_H
