#include <QtSql>
#include <QtDebug>

#include "library/trackcollection.h"

#ifdef __SQLITE3__
#include <sqlite3.h>
#endif

#include "library/librarytablemodel.h"
#include "library/schemamanager.h"
#include "trackinfoobject.h"
#include "xmlparse.h"
#include "util/assert.h"

// static
const int TrackCollection::kRequiredSchemaVersion = 24;

TrackCollection::TrackCollection(ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig),
          m_db(QSqlDatabase::addDatabase("QSQLITE")), // defaultConnection
          m_playlistDao(m_db),
          m_crateDao(m_db),
          m_cueDao(m_db),
          m_directoryDao(m_db),
          m_analysisDao(m_db, pConfig),
          m_libraryHashDao(m_db),
          m_trackDao(m_db, m_cueDao, m_playlistDao, m_crateDao,
                     m_analysisDao, m_libraryHashDao, pConfig) {
    qDebug() << "Available QtSQL drivers:" << QSqlDatabase::drivers();

    m_db.setHostName("localhost");
    m_db.setDatabaseName(pConfig->getSettingsPath().append("/mixxxdb.sqlite"));
    m_db.setUserName("mixxx");
    m_db.setPassword("mixxx");
    bool ok = m_db.open();
    qDebug() << "DB status:" << m_db.databaseName() << "=" << ok;
    if (m_db.lastError().isValid()) {
        qDebug() << "Error loading database:" << m_db.lastError();
    }
    // Check for tables and create them if missing
    if (!checkForTables()) {
        // TODO(XXX) something a little more elegant
        exit(-1);
    }
}

TrackCollection::~TrackCollection() {
    qDebug() << "~TrackCollection()";
    m_trackDao.finish();

    if (m_db.isOpen()) {
        // There should never be an outstanding transaction when this code is
        // called. If there is, it means we probably aren't committing a
        // transaction somewhere that should be.
        if (m_db.rollback()) {
            qDebug() << "ERROR: There was a transaction in progress on the main database connection while shutting down."
                    << "There is a logic error somewhere.";
        }
        m_db.close();
    } else {
        qDebug() << "ERROR: The main database connection was closed before TrackCollection closed it."
                << "There is a logic error somewhere.";
    }
}

bool TrackCollection::checkForTables() {
    if (!m_db.open()) {
        QMessageBox::critical(0, tr("Cannot open database"),
                            tr("Unable to establish a database connection.\n"
                                "Mixxx requires QT with SQLite support. Please read "
                                "the Qt SQL driver documentation for information on how "
                                "to build it.\n\n"
                                "Click OK to exit."), QMessageBox::Ok);
        return false;
    }

#ifdef __SQLITE3__
    installSorting(m_db);
#endif

    // The schema XML is baked into the binary via Qt resources.
    QString schemaFilename(":schema.xml");
    QString okToExit = tr("Click OK to exit.");
    QString upgradeFailed = tr("Cannot upgrade database schema");
    QString upgradeToVersionFailed = tr("Unable to upgrade your database schema to version %1")
            .arg(QString::number(kRequiredSchemaVersion));
    QString helpEmail = tr("For help with database issues contact "
                           "mixxx-devel@lists.sourceforge.net.");

    SchemaManager::Result result = SchemaManager::upgradeToSchemaVersion(
            schemaFilename, m_db, kRequiredSchemaVersion);
    switch (result) {
        case SchemaManager::RESULT_BACKWARDS_INCOMPATIBLE:
            QMessageBox::warning(
                    0, upgradeFailed,
                    upgradeToVersionFailed + "\n" +
                    tr("Your mixxxdb.sqlite file was created by a newer "
                       "version of Mixxx and is incompatible.") +
                    "\n\n" + okToExit,
                    QMessageBox::Ok);
            return false;
        case SchemaManager::RESULT_UPGRADE_FAILED:
            QMessageBox::warning(
                    0, upgradeFailed,
                    upgradeToVersionFailed + "\n" +
                    tr("Your mixxxdb.sqlite file may be corrupt.") + "\n" +
                    tr("Try renaming it and restarting Mixxx.") + "\n" +
                    helpEmail + "\n\n" + okToExit,
                    QMessageBox::Ok);
            return false;
        case SchemaManager::RESULT_SCHEMA_ERROR:
            QMessageBox::warning(
                    0, upgradeFailed,
                    upgradeToVersionFailed + "\n" +
                    tr("The database schema file is invalid.") + "\n" +
                    helpEmail + "\n\n" + okToExit,
                    QMessageBox::Ok);
            return false;
        case SchemaManager::RESULT_OK:
        default:
            break;
    }

    m_trackDao.initialize();
    m_playlistDao.initialize();
    m_crateDao.initialize();
    m_cueDao.initialize();
    m_directoryDao.initialize();
    m_libraryHashDao.initialize();
    return true;
}

QSqlDatabase& TrackCollection::getDatabase() {
    return m_db;
}

CrateDAO& TrackCollection::getCrateDAO() {
    return m_crateDao;
}

TrackDAO& TrackCollection::getTrackDAO() {
    return m_trackDao;
}

PlaylistDAO& TrackCollection::getPlaylistDAO() {
    return m_playlistDao;
}

DirectoryDAO& TrackCollection::getDirectoryDAO() {
    return m_directoryDao;
}

QSharedPointer<BaseTrackCache> TrackCollection::getTrackSource() {
    return m_defaultTrackSource;
}

void TrackCollection::setTrackSource(QSharedPointer<BaseTrackCache> trackSource) {
    DEBUG_ASSERT_AND_HANDLE(m_defaultTrackSource.isNull()) {
        return;
    }
    m_defaultTrackSource = trackSource;
}

#ifdef __SQLITE3__
// from public domain code
// http://www.archivum.info/qt-interest@trolltech.com/2008-12/00584/Re-%28Qt-interest%29-Qt-Sqlite-UserDefinedFunction.html
void TrackCollection::installSorting(QSqlDatabase &db) {
    QVariant v = db.driver()->handle();
    if (v.isValid() && strcmp(v.typeName(), "sqlite3*") == 0) {
        // v.data() returns a pointer to the handle
        sqlite3* handle = *static_cast<sqlite3**>(v.data());
        if (handle != 0) { // check that it is not NULL
            int result = sqlite3_create_collation(
                    handle,
                    "localeAwareCompare",
                    SQLITE_UTF16,
                    NULL,
                    sqliteLocaleAwareCompare);
            if (result != SQLITE_OK)
            qWarning() << "Could not add string collation function: " << result;

            result = sqlite3_create_function(
                    handle,
                    "like",
                    2,
                    SQLITE_ANY,
                    NULL,
                    sqliteLike,
                    NULL, NULL);
            if (result != SQLITE_OK)
            qWarning() << "Could not add like 2 function: " << result;

            result = sqlite3_create_function(
                    handle,
                    "like",
                    3,
                    SQLITE_UTF8, // No conversion, Data is stored as UTF8
                    NULL,
                    sqliteLike,
                    NULL, NULL);
            if (result != SQLITE_OK)
            qWarning() << "Could not add like 3 function: " << result;
        } else {
            qWarning() << "Could not get sqlite handle";
        }
    } else {
        qWarning() << "handle variant returned typename " << v.typeName();
    }
}

// The collating function callback is invoked with a copy of the pArg
// application data pointer and with two strings in the encoding specified
// by the eTextRep argument.
// The collating function must return an integer that is negative, zero,
// or positive if the first string is less than, equal to, or greater
// than the second, respectively.
//static
int TrackCollection::sqliteLocaleAwareCompare(void* pArg,
                                              int len1, const void* data1,
                                              int len2, const void* data2) {
    Q_UNUSED(pArg);
    // Construct a QString without copy
    QString string1 = QString::fromRawData(reinterpret_cast<const QChar*>(data1),
                                           len1 / sizeof(QChar));
    QString string2 = QString::fromRawData(reinterpret_cast<const QChar*>(data2),
                                           len2 / sizeof(QChar));
    return QString::localeAwareCompare(string1, string2);
}

// This implements the like() SQL function. This is used by the LIKE operator.
// The SQL statement 'A LIKE B' is implemented as 'like(B, A)', and if there is
// an escape character, say E, it is implemented as 'like(B, A, E)'
//static
void TrackCollection::sqliteLike(sqlite3_context *context,
                                int aArgc,
                                sqlite3_value **aArgv) {
    DEBUG_ASSERT_AND_HANDLE(aArgc == 2 || aArgc == 3) {
        return;
    }

    const char* b = reinterpret_cast<const char*>(
            sqlite3_value_text(aArgv[0]));
    const char* a = reinterpret_cast<const char*>(
            sqlite3_value_text(aArgv[1]));

    if (!a || !b) {
        return;
    }

    QString stringB = QString::fromUtf8(b); // Like String
    QString stringA = QString::fromUtf8(a);

    QChar esc = '\0'; // Escape
    if (aArgc == 3) {
        const char* e = reinterpret_cast<const char*>(
                sqlite3_value_text(aArgv[2]));
        if(e) {
            QString stringE = QString::fromUtf8(e);
            if (!stringE.isEmpty()) {
                esc = stringE.data()[0];
            }
        }
    }

    int ret = likeCompareLatinLow(&stringB, &stringA, esc);
    sqlite3_result_int64(context, ret);
    return;
}

//static
void TrackCollection::makeLatinLow(QChar* c, int count) {
    for (int i = 0; i < count; ++i) {
        if (c[i].decompositionTag() != QChar::NoDecomposition) {
            c[i] = c[i].decomposition()[0];
        }
        if (c[i].isUpper()) {
            c[i] = c[i].toLower();
        }
    }
}

//static
int TrackCollection::likeCompareLatinLow(
        QString* pattern,
        QString* string,
        const QChar esc) {
    makeLatinLow(pattern->data(), pattern->length());
    makeLatinLow(string->data(), string->length());
    //qDebug() << *pattern << *string;
    return likeCompareInner(pattern->data(), pattern->length(), string->data(), string->length(), esc);
}

// Compare two strings for equality where the first string is
// a "LIKE" expression. Return true (1) if they are the same and
// false (0) if they are different.
// This is the original sqlite3 icuLikeCompare rewritten for QChar
//static
int TrackCollection::likeCompareInner(
  const QChar* pattern, // LIKE pattern
  int patternSize,
  const QChar* string, // The string to compare against
  int stringSize,
  const QChar esc // The escape character
) {
    static const QChar MATCH_ONE = QChar('_');
    static const QChar MATCH_ALL = QChar('%');

    int iPattern = 0; // Current index in pattern
    int iString = 0; // Current index in string

    bool prevEscape = false; // True if the previous character was uEsc

    while (iPattern < patternSize) {
        // Read (and consume) the next character from the input pattern.
        QChar uPattern = pattern[iPattern++];
        // There are now 4 possibilities:
        // 1. uPattern is an unescaped match-all character "%",
        // 2. uPattern is an unescaped match-one character "_",
        // 3. uPattern is an unescaped escape character, or
        // 4. uPattern is to be handled as an ordinary character

        if (!prevEscape && uPattern == MATCH_ALL) {
            // Case 1.
            QChar c;

            // Skip any MATCH_ALL or MATCH_ONE characters that follow a
            // MATCH_ALL. For each MATCH_ONE, skip one character in the
            // test string.

            if (iPattern >= patternSize) {
                // Tailing %
                return 1;
            }

            while ((c = pattern[iPattern]) == MATCH_ALL || c == MATCH_ONE) {
                if (c == MATCH_ONE) {
                    if (++iString == stringSize) {
                        return 0;
                    }
                }
                if (++iPattern == patternSize) {
                    // Two or more tailing %
                    return 1;
                }
            }

            while (iString < stringSize) {
                if (likeCompareInner(&pattern[iPattern], patternSize - iPattern,
                                &string[iString], stringSize - iString, esc)) {
                    return 1;
                }
                iString++;
            }
            return 0;
        } else if (!prevEscape && uPattern == MATCH_ONE) {
            // Case 2.
            if (++iString == stringSize) {
                return 0;
            }
        } else if (!prevEscape && uPattern == esc) {
            // Case 3.
            prevEscape = 1;
        } else {
            // Case 4.
            if (iString == stringSize) {
                return 0;
            }
            QChar uString = string[iString++];
            if (uString != uPattern) {
                return 0;
            }
            prevEscape = false;
        }
    }
    return iString == stringSize;
}



/*
static int
likeCompare(nsAString::const_iterator aPatternItr,
            nsAString::const_iterator aPatternEnd,
            nsAString::const_iterator aStringItr,
            nsAString::const_iterator aStringEnd,
            PRUnichar aEscape)
{
  const PRUnichar MATCH_ALL('%');
  const PRUnichar MATCH_ONE('_');

  PRBool lastWasEscape = PR_FALSE;
  while (aPatternItr != aPatternEnd) {

* What we do in here is take a look at each character from the input
* pattern, and do something with it. There are 4 possibilities:
* 1) character is an un-escaped match-all character
* 2) character is an un-escaped match-one character
* 3) character is an un-escaped escape character
* 4) character is not any of the above

    if (!lastWasEscape && *aPatternItr == MATCH_ALL) {
      // CASE 1

* Now we need to skip any MATCH_ALL or MATCH_ONE characters that follow a
* MATCH_ALL character. For each MATCH_ONE character, skip one character
* in the pattern string.

      while (*aPatternItr == MATCH_ALL || *aPatternItr == MATCH_ONE) {
        if (*aPatternItr == MATCH_ONE) {
          // If we've hit the end of the string we are testing, no match
          if (aStringItr == aStringEnd)
            return 0;
          aStringItr++;
        }
        aPatternItr++;
      }

      // If we've hit the end of the pattern string, match
      if (aPatternItr == aPatternEnd)
        return 1;

      while (aStringItr != aStringEnd) {
        if (likeCompare(aPatternItr, aPatternEnd, aStringItr, aStringEnd, aEscape)) {
          // we've hit a match, so indicate this
          return 1;
        }
        aStringItr++;
      }

      // No match
      return 0;
    } else if (!lastWasEscape && *aPatternItr == MATCH_ONE) {
      // CASE 2
      if (aStringItr == aStringEnd) {
        // If we've hit the end of the string we are testing, no match
        return 0;
      }
      aStringItr++;
      lastWasEscape = PR_FALSE;
    } else if (!lastWasEscape && *aPatternItr == aEscape) {
      // CASE 3
      lastWasEscape = PR_TRUE;
    } else {
      // CASE 4
      if (ToUpperCase(*aStringItr) != ToUpperCase(*aPatternItr)) {
        // If we've hit a point where the strings don't match, there is no match
        return 0;
      }
      aStringItr++;
      lastWasEscape = PR_FALSE;
    }

    aPatternItr++;
  }

  return aStringItr == aStringEnd;
}


.
void
StorageUnicodeFunctions::likeFunction(sqlite3_context *p,
                                      int aArgc,
                                      sqlite3_value **aArgv)
{
  NS_ASSERTION(2 == aArgc || 3 == aArgc, "Invalid number of arguments!");

  if (sqlite3_value_bytes(aArgv[0]) > SQLITE_MAX_LIKE_PATTERN_LENGTH) {
    sqlite3_result_error(p, "LIKE or GLOB pattern too complex", SQLITE_TOOBIG);
    return;
  }

  if (!sqlite3_value_text16(aArgv[0]) || !sqlite3_value_text16(aArgv[1]))
    return;

  nsDependentString A(static_cast<const PRUnichar *>(sqlite3_value_text16(aArgv[1])));
  nsDependentString B(static_cast<const PRUnichar *>(sqlite3_value_text16(aArgv[0])));
  NS_ASSERTION(!B.IsEmpty(), "LIKE string must not be null!");

  PRUnichar E = 0;
  if (3 == aArgc)
    E = static_cast<const PRUnichar *>(sqlite3_value_text16(aArgv[2]))[0];

  nsAString::const_iterator itrString, endString;
  A.BeginReading(itrString);
  A.EndReading(endString);
  nsAString::const_iterator itrPattern, endPattern;
  B.BeginReading(itrPattern);
  B.EndReading(endPattern);
  sqlite3_result_int(p, likeCompare(itrPattern, endPattern,
                                    itrString, endString, E));
}
*/
#endif
