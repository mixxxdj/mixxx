#ifndef THREADDAO_H
#define THREADDAO_H

#include <QThread>
#include <QSqlDatabase>

#include "dao.h"
#include "configobject.h"
#include "library/dao/trackdao.h"
#include "library/dao/cratedao.h"
#include "library/dao/cuedao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"

// Lambda function
typedef std::function <void ()> func;

class QWidget;

// tro: Class for separate thread. It will do empty cycles until got some work.
//      All database access must be done trough this thread.
class ThreadDAO : public QThread {
    Q_OBJECT

    func m_lambda;
    ConfigObject<ConfigValue>* m_pConfig;
    QSqlDatabase m_database;
    PlaylistDAO m_playlistDao;
    CrateDAO m_crateDao;
    CueDAO m_cueDao;
    AnalysisDao m_analysisDao;
    TrackDAO m_trackDao;

    bool m_haveFunction;
    bool m_callFinished;
    bool m_stop;

    QString m_databasePath;

public:
    explicit ThreadDAO(ConfigObject<ConfigValue>* pConfig, QObject *parent = 0);
    ~ThreadDAO();
    void run();
    void callSync(func lambda, QWidget& w);
    void stopThread();
    void setLambda(func lambda);
    bool checkForTables();
    // Import the files in a given diretory, without recursing into subdirectories
    bool importDirectory(const QString& directory, TrackDAO& trackDao,
                         const QStringList& nameFilters, volatile bool* cancel);


    // Getters
    QSqlDatabase& getDatabase() { return m_database; }
    CrateDAO& getCrateDAO() { return m_crateDao; }
    TrackDAO& getTrackDAO() { return m_trackDao; }
    PlaylistDAO& getPlaylistDAO() { return m_playlistDao; }
    ConfigObject<ConfigValue>* getConfig() { return m_pConfig; }

};

#endif // THREADDAO_H
