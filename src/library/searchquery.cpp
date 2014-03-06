#include <QtDebug>

#include "library/searchquery.h"

#include "library/queryutil.h"
#include "track/keyutils.h"
#include "library/dao/trackdao.h"

QVariant getTrackValueForColumn(const TrackPointer& pTrack, const QString& column) {
    if (column == LIBRARYTABLE_ARTIST) {
        return pTrack->getArtist();
    } else if (column == LIBRARYTABLE_TITLE) {
        return pTrack->getTitle();
    } else if (column == LIBRARYTABLE_ALBUM) {
        return pTrack->getAlbum();
    } else if (column == LIBRARYTABLE_ALBUMARTIST) {
        return pTrack->getAlbumArtist();
    } else if (column == LIBRARYTABLE_YEAR) {
        return pTrack->getYear();
    } else if (column == LIBRARYTABLE_DATETIMEADDED) {
        return pTrack->getDateAdded();
    } else if (column == LIBRARYTABLE_GENRE) {
        return pTrack->getGenre();
    } else if (column == LIBRARYTABLE_COMPOSER) {
        return pTrack->getComposer();
    } else if (column == LIBRARYTABLE_GROUPING) {
        return pTrack->getGrouping();
    } else if (column == LIBRARYTABLE_FILETYPE) {
        return pTrack->getType();
    } else if (column == LIBRARYTABLE_TRACKNUMBER) {
        return pTrack->getTrackNumber();
    } else if (column == LIBRARYTABLE_LOCATION) {
        return pTrack->getLocation();
    } else if (column == LIBRARYTABLE_COMMENT) {
        return pTrack->getComment();
    } else if (column == LIBRARYTABLE_DURATION) {
        return pTrack->getDuration();
    } else if (column == LIBRARYTABLE_BITRATE) {
        return pTrack->getBitrate();
    } else if (column == LIBRARYTABLE_BPM) {
        return pTrack->getBpm();
    } else if (column == LIBRARYTABLE_PLAYED) {
        return pTrack->getPlayed();
    } else if (column == LIBRARYTABLE_TIMESPLAYED) {
        return pTrack->getTimesPlayed();
    } else if (column == LIBRARYTABLE_RATING) {
        return pTrack->getRating();
    } else if (column == LIBRARYTABLE_KEY) {
        return pTrack->getKeyText();
    } else if (column == LIBRARYTABLE_KEY_ID) {
        return static_cast<int>(pTrack->getKey());
    } else if (column == LIBRARYTABLE_BPM_LOCK) {
        return pTrack->hasBpmLock();
    }

    return QVariant();
}

bool AndNode::match(const TrackPointer& pTrack) const {
    if (m_nodes.isEmpty()) {
        return false;
    }

    foreach (const QueryNode* pNode, m_nodes) {
        if (!pNode->match(pTrack)) {
            return false;
        }
    }
    return true;
}

QString AndNode::toSql() const {
    QStringList queryFragments;
    foreach (const QueryNode* pNode, m_nodes) {
        QString sql = pNode->toSql();
        if (!sql.isEmpty()) {
            queryFragments << sql;
        }
    }
    return queryFragments.join(" AND ");
}

bool OrNode::match(const TrackPointer& pTrack) const {
    foreach (const QueryNode* pNode, m_nodes) {
        if (pNode->match(pTrack)) {
            return true;
        }
    }
    return false;
}

QString OrNode::toSql() const {
    QStringList queryFragments;
    foreach (const QueryNode* pNode, m_nodes) {
        QString sql = pNode->toSql();
        if (!sql.isEmpty()) {
            queryFragments << sql;
        }
    }
    return queryFragments.join(" OR ");
}

bool TextFilterNode::match(const TrackPointer& pTrack) const {
    foreach (QString sqlColumn, m_sqlColumns) {
        QVariant value = getTrackValueForColumn(pTrack, sqlColumn);
        if (!value.isValid() || !qVariantCanConvert<QString>(value)) {
            continue;
        }

        if (value.toString().contains(m_argument, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

QString TextFilterNode::toSql() const {
    FieldEscaper escaper(m_database);
    QString escapedArgument = escaper.escapeString("%" + m_argument + "%");

    QStringList searchClauses;
    foreach (QString sqlColumn, m_sqlColumns) {
        searchClauses << QString("(%1 LIKE %2)").arg(sqlColumn, escapedArgument);
    }

    return searchClauses.length() > 1 ?
            QString("(%1)").arg(searchClauses.join(" OR ")) :
            searchClauses.at(0);
}

NumericFilterNode::NumericFilterNode(const QStringList& sqlColumns,
                                     QString argument)
        : m_sqlColumns(sqlColumns),
          m_bOperatorQuery(false),
          m_operator("="),
          m_dOperatorArgument(0.0),
          m_bRangeQuery(false),
          m_dRangeLow(0.0),
          m_dRangeHigh(0.0) {
    QRegExp operatorMatcher("^(>|>=|=|<|<=)(.*)$");
    if (operatorMatcher.indexIn(argument) != -1) {
        m_operator = operatorMatcher.cap(1);
        argument = operatorMatcher.cap(2);
    }
	//first check if the query is about duration
    if(sqlColumns.at(0)=="duration"){

        //if it is duration then humanreadable time will be converted to computable time(second)
        argument=parseHumanReadableTime(argument);
    }


    bool parsed = false;
    // Try to convert to see if it parses.
    m_dOperatorArgument = argument.toDouble(&parsed);
    if (parsed) {
        m_bOperatorQuery = true;
    }

    QStringList rangeArgs = argument.split("-");
    if (rangeArgs.length() == 2) {
        bool lowOk = false;
        m_dRangeLow = rangeArgs[0].toDouble(&lowOk);
        bool highOk = false;
        m_dRangeHigh = rangeArgs[1].toDouble(&highOk);

        if (lowOk && highOk && m_dRangeLow <= m_dRangeHigh) {
            m_bRangeQuery = true;
        }
    }

}
QString NumericFilterNode::parseHumanReadableTime(QString durationHumanReadable)
{

    QStringList args=durationHumanReadable.split("-");
    qDebug()<<durationHumanReadable;
    if(args.length()==1){
        qDebug()<<"only one parameter";
        return getTimeInHMS(durationHumanReadable);
    }else if(args.length()==2){        
         qDebug()<<"two parameter";
         return getTimeInHMS(args[0])+"-"+getTimeInHMS(args[1]);
    }else{
        qDebug()<<"undefind";
        return durationHumanReadable;
    }

}

QString NumericFilterNode::getTimeInHMS(QString durationHumanReadable)
{

    bool parseable=false;
    double totalTime=-1;
    totalTime= durationHumanReadable.toDouble(&parseable);
    if(parseable)
    {
        return  durationHumanReadable;
    }

    QString tempStr="";
    bool hour=false;
    bool miniute=false;
    bool second=false;

    for(int i = 0;i< durationHumanReadable.length();i++)
    {
        if((durationHumanReadable.at(i) == 'h'
                ||  durationHumanReadable.at(i) == 'H'
                ||  durationHumanReadable.at(i) == 'm'
                ||  durationHumanReadable.at(i) == 'M'
                ||  durationHumanReadable.at(i) == 's'
                ||  durationHumanReadable.at(i) == 'S')
                && !tempStr.isEmpty()){
            bool parsed = false;
                // Try to convert to see if it parses.
            double dbl= tempStr.trimmed().toDouble(&parsed);
            if((durationHumanReadable.at(i) == 'h'
                    || durationHumanReadable.at(i) == 'H')
                    && parsed
                    && !hour){
                    totalTime+= dbl*3600;
                    hour=true;
            }

            if((durationHumanReadable.at(i) == 'm'
                    || durationHumanReadable.at(i) == 'M')
                    && parsed
                    && !miniute){
                    totalTime+= dbl*60;
                    miniute=true;
            }

            if((durationHumanReadable.at(i) == 's'
                    || durationHumanReadable.at(i)== 'S')
                    && parsed
                    && !second){
                    totalTime+= dbl;
                    second=true;
            }
            qDebug()<<dbl<<durationHumanReadable.at(i);
                tempStr = "";
        }else{

            tempStr.append(durationHumanReadable.at(i));
        }


    }

    //if there is no Hour/miniute/second identification then it will taken as second
    if(!tempStr.isEmpty() && !second){
        bool parsed = false;
        double dbl = tempStr.trimmed().toDouble(&parsed);
        if(parsed){

                totalTime+=dbl;
        }
    }

    qDebug()<<totalTime;
    return QString::number(totalTime);
}

bool NumericFilterNode::match(const TrackPointer& pTrack) const {
    foreach (QString sqlColumn, m_sqlColumns) {
        QVariant value = getTrackValueForColumn(pTrack, sqlColumn);
        if (!value.isValid() || !qVariantCanConvert<double>(value)) {
            continue;
        }

        double dValue = value.toDouble();
        if (m_bOperatorQuery) {
            if ((m_operator == "=" && dValue == m_dOperatorArgument) ||
                (m_operator == "<" && dValue < m_dOperatorArgument) ||
                (m_operator == ">" && dValue > m_dOperatorArgument) ||
                (m_operator == "<=" && dValue <= m_dOperatorArgument) ||
                (m_operator == ">=" && dValue >= m_dOperatorArgument)) {
                return true;
            }
        } else if (m_bRangeQuery && dValue >= m_dRangeLow &&
                   dValue <= m_dRangeHigh) {
            return true;
        }
    }
    return false;
}

QString NumericFilterNode::toSql() const {
    if (m_bOperatorQuery) {
        QStringList searchClauses;
        foreach (const QString& sqlColumn, m_sqlColumns) {
            searchClauses << QString("(%1 %2 %3)").arg(
                sqlColumn, m_operator, QString::number(m_dOperatorArgument));
        }
        return searchClauses.length() > 1 ?
                QString("(%1)").arg(searchClauses.join(" OR ")) :
                searchClauses[0];
    }

    if (m_bRangeQuery) {
        QStringList searchClauses;
        foreach (const QString& sqlColumn, m_sqlColumns) {
            searchClauses << QString("(%1 >= %2 AND %1 <= %3)")
                    .arg(sqlColumn, QString::number(m_dRangeLow),
                         QString::number(m_dRangeHigh));
        }

        return searchClauses.length() > 1 ?
                QString("(%1)").arg(searchClauses.join(" OR ")) :
                searchClauses[0];
    }

    return QString();
}

KeyFilterNode::KeyFilterNode(mixxx::track::io::key::ChromaticKey key,
                             bool fuzzy) {
    if (fuzzy) {
        m_matchKeys = KeyUtils::getCompatibleKeys(key);
    } else {
        m_matchKeys.push_back(key);
    }
}

bool KeyFilterNode::match(const TrackPointer& pTrack) const {
    return m_matchKeys.contains(pTrack->getKey());
}

QString KeyFilterNode::toSql() const {
    QStringList searchClauses;
    foreach (mixxx::track::io::key::ChromaticKey match, m_matchKeys) {
        searchClauses << QString("(key_id IS %1)").arg(QString::number(match));
    }

    return searchClauses.length() > 1 ?
            QString("(%1)").arg(searchClauses.join(" OR ")) :
            searchClauses[0];
}
