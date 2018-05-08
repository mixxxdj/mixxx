#include "util/db/dbconnection.h"

#include <QDir>
#include <QSqlDriver>
#include <QSqlError>

#ifdef __SQLITE3__
#include <sqlite3.h>
#endif // __SQLITE3__

#include "util/db/sqllikewildcards.h"

#include "util/string.h"
#include "util/assert.h"


// Originally from public domain code:
// http://www.archivum.info/qt-interest@trolltech.com/2008-12/00584/Re-%28Qt-interest%29-Qt-Sqlite-UserDefinedFunction.html

namespace {

const QString kDatabaseType = "QSQLITE";

const QString kDatabaseHostName = "localhost";
const QString kDatabaseFileName = "mixxxdb.sqlite";
const QString kDatabaseUserName = "mixxx";
const QString kDatabasePassword = "mixxx";

void makeLatinLow(QChar* c, int count) {
    for (int i = 0; i < count; ++i) {
        if (c[i].decompositionTag() != QChar::NoDecomposition) {
            c[i] = c[i].decomposition()[0];
        }
        if (c[i].isUpper()) {
            c[i] = c[i].toLower();
        }
    }
}

const QChar kSqlLikeEscapeDefault = '\0';

// The collating function callback is invoked with a copy of the pArg
// application data pointer and with two strings in the encoding specified
// by the eTextRep argument.
// The collating function must return an integer that is negative, zero,
// or positive if the first string is less than, equal to, or greater
// than the second, respectively.
int sqliteStringCompareUTF16(void* pArg,
                        int len1, const void* data1,
                        int len2, const void* data2) {
    Q_UNUSED(pArg);
    // Construct a QString without copy
    QString string1 = QString::fromRawData(reinterpret_cast<const QChar*>(data1),
                                           len1 / sizeof(QChar));
    QString string2 = QString::fromRawData(reinterpret_cast<const QChar*>(data2),
                                           len2 / sizeof(QChar));
    return compareLocalAwareCaseInsensitive(string1, string2);
}

// Compare two strings for equality where the first string is
// a "LIKE" expression. Return true (1) if they are the same and
// false (0) if they are different.
// This is the original sqlite3 icuLikeCompare rewritten for QChar
int likeCompareInner(
    const QChar* pattern, // LIKE pattern
    int patternSize,
    const QChar* string, // The string to compare against
    int stringSize,
    const QChar esc) { // The escape character

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

        if (!prevEscape && uPattern == kSqlLikeMatchAll) {
            // Case 1.
            QChar c;

            // Skip any kSqlLikeMatchAll or kSqlLikeMatchOne characters that follow a
            // kSqlLikeMatchAll. For each kSqlLikeMatchOne, skip one character in the
            // test string.

            if (iPattern >= patternSize) {
                // Tailing %
                return 1;
            }

            while ((c = pattern[iPattern]) == kSqlLikeMatchAll || c == kSqlLikeMatchOne) {
                if (c == kSqlLikeMatchOne) {
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
        } else if (!prevEscape && uPattern == kSqlLikeMatchOne) {
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

#ifdef __SQLITE3__

const char* const kLexicographicalCollationFunc = "mixxxLexicographicalCollation";

// This implements the like() SQL function. This is used by the LIKE operator.
// The SQL statement 'A LIKE B' is implemented as 'like(B, A)', and if there is
// an escape character, say E, it is implemented as 'like(B, A, E)'
//static
void sqliteLike(sqlite3_context *context,
                                int aArgc,
                                sqlite3_value **aArgv) {
    VERIFY_OR_DEBUG_ASSERT(aArgc == 2 || aArgc == 3) {
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

    QChar esc = kSqlLikeEscapeDefault;
    if (aArgc == 3) {
        const char* e = reinterpret_cast<const char*>(
                sqlite3_value_text(aArgv[2]));
        if (e) {
            QString stringE = QString::fromUtf8(e);
            if (!stringE.isEmpty()) {
                esc = stringE.data()[0];
            }
        }
    }

    int ret = DbConnection::likeCompareLatinLow(&stringB, &stringA, esc);
    sqlite3_result_int64(context, ret);
    return;
}

#endif // __SQLITE3__

} // anonymous namespace

DbConnection::DbConnection(const QString& dirPath)
    : m_filePath(QDir(dirPath).filePath(kDatabaseFileName)),
      m_database(QSqlDatabase::addDatabase(kDatabaseType)) {
    qDebug()
        << "Available drivers for database connection:"
        << QSqlDatabase::drivers();

    m_database.setHostName(kDatabaseHostName);
    m_database.setDatabaseName(m_filePath);
    m_database.setUserName(kDatabaseUserName);
    m_database.setPassword(kDatabasePassword);
    if (!m_database.open()) {
        qWarning() << "Failed to open database connection:"
            << *this
            << m_database.lastError();
        DEBUG_ASSERT(!*this); // failure
        return; // early exit
    }

    QVariant v = m_database.driver()->handle();
    VERIFY_OR_DEBUG_ASSERT(v.isValid()) {
        return; // early exit
    }
#ifdef __SQLITE3__
    if (strcmp(v.typeName(), "sqlite3*") == 0) {
        // v.data() returns a pointer to the handle
        sqlite3* handle = *static_cast<sqlite3**>(v.data());
        VERIFY_OR_DEBUG_ASSERT(handle != nullptr) {
            qWarning() << "Could not get sqlite3 handle";
            m_database.close();
            DEBUG_ASSERT(!*this); // failure
            return; // early exit
        }

        int result = sqlite3_create_collation(
                        handle,
                        kLexicographicalCollationFunc,
                        SQLITE_UTF16,
                        nullptr,
                        sqliteStringCompareUTF16);
        VERIFY_OR_DEBUG_ASSERT(result == SQLITE_OK) {
            qWarning() << "Failed to install lexicographical collation function:" << result;
        }

        result = sqlite3_create_function(
                        handle,
                        "like",
                        2,
                        SQLITE_ANY,
                        nullptr,
                        sqliteLike,
                        nullptr, nullptr);
        VERIFY_OR_DEBUG_ASSERT(result == SQLITE_OK) {
            qWarning() << "Failed to install like 2 function:" << result;
        }

        result = sqlite3_create_function(
                        handle,
                        "like",
                        3,
                        SQLITE_UTF8, // No conversion, Data is stored as UTF8
                        nullptr,
                        sqliteLike,
                        nullptr, nullptr);
        VERIFY_OR_DEBUG_ASSERT(result == SQLITE_OK) {
            qWarning() << "Failed to install like 3 function:" << result;
        }

        DEBUG_ASSERT(*this); // success
        return; // early exit
    } else {
        qWarning() << "localecompare requires a SQLite3 database driver, found:"
                   << v.typeName();
    }
#endif // __SQLITE3__
}

DbConnection::~DbConnection() {
    VERIFY_OR_DEBUG_ASSERT(*this) {
        qWarning()
            << "Database connection has already been closed:"
            << *this;
        return; // early exit
    }
    // There should never be an outstanding transaction when this code is
    // called. If there is, it means we probably aren't committing a
    // transaction somewhere that should be.
    VERIFY_OR_DEBUG_ASSERT(!m_database.rollback()) {
        qWarning()
            << "Rolled back open transaction before closing database connection:"
            << *this;
    }
    DEBUG_ASSERT(*this);
    qDebug()
        << "Closing database connection:"
        << *this;
    m_database.close();
    DEBUG_ASSERT(!*this);
}

QDebug operator<<(QDebug debug, const DbConnection& dbConnection) {
    return debug << kDatabaseType << dbConnection.m_filePath;
}

//static
QString DbConnection::collateLexicographically(const QString& orderByQuery) {
#ifdef __SQLITE3__
        return orderByQuery + QString(" COLLATE %1").arg(kLexicographicalCollationFunc);
#else
        return orderByQuery;
#endif //  __SQLITE3__
}

//static
int DbConnection::likeCompareLatinLow(
        QString* pattern,
        QString* string,
        QChar esc) {
    makeLatinLow(pattern->data(), pattern->length());
    makeLatinLow(string->data(), string->length());
    return likeCompareInner(
            pattern->data(), pattern->length(),
            string->data(), string->length(),
            esc);
}
