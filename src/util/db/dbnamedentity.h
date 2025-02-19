#pragma once

#include "util/db/dbentity.h"

// Base class for database entities with a non-empty name.
template<typename T> // where T is derived from DbId
class DbNamedEntity: public DbEntity<T> {
  public:
    ~DbNamedEntity() override = default;

    bool hasName() const {
        return !m_name.isEmpty();
    }
    const QString& getName() const {
        return m_name;
    }
    void setName(QString name) {
        // Due to missing trimming names with only whitespaces
        // may occur in the database and can't we assert on
        // this here!
        DEBUG_ASSERT(!name.isEmpty());
        m_name = std::move(name);
    }
    void resetName() {
        m_name.clear();
        DEBUG_ASSERT(!hasName());
    }

    // Next lines are for duplicating samrties
    const QString& getSearchInput() const {
        return m_searchInput;
    }
    const QString& getSearchSql() const {
        return m_searchSql;
    }

    void setSearchInput(QString searchInput) {
        //        DEBUG_ASSERT(!searchInput.isEmpty());
        m_searchInput = std::move(searchInput);
    }
    void setSearchSql(QString searchSql) {
        //        DEBUG_ASSERT(!searchSql.isEmpty());
        m_searchSql = std::move(searchSql);
    }

    // Conditions
    const QString& getCondition1Field() const {
        return m_Condition1Field;
    }
    const QString& getCondition2Field() const {
        return m_Condition2Field;
    }
    const QString& getCondition3Field() const {
        return m_Condition3Field;
    }
    const QString& getCondition4Field() const {
        return m_Condition4Field;
    }
    const QString& getCondition5Field() const {
        return m_Condition5Field;
    }
    const QString& getCondition6Field() const {
        return m_Condition6Field;
    }
    const QString& getCondition7Field() const {
        return m_Condition7Field;
    }
    const QString& getCondition8Field() const {
        return m_Condition8Field;
    }
    const QString& getCondition9Field() const {
        return m_Condition9Field;
    }
    const QString& getCondition10Field() const {
        return m_Condition10Field;
    }
    const QString& getCondition11Field() const {
        return m_Condition11Field;
    }
    const QString& getCondition12Field() const {
        return m_Condition12Field;
    }
    const QString& getCondition1Operator() const {
        return m_Condition1Operator;
    }
    const QString& getCondition2Operator() const {
        return m_Condition2Operator;
    }
    const QString& getCondition3Operator() const {
        return m_Condition3Operator;
    }
    const QString& getCondition4Operator() const {
        return m_Condition4Operator;
    }
    const QString& getCondition5Operator() const {
        return m_Condition5Operator;
    }
    const QString& getCondition6Operator() const {
        return m_Condition6Operator;
    }
    const QString& getCondition7Operator() const {
        return m_Condition7Operator;
    }
    const QString& getCondition8Operator() const {
        return m_Condition8Operator;
    }
    const QString& getCondition9Operator() const {
        return m_Condition9Operator;
    }
    const QString& getCondition10Operator() const {
        return m_Condition10Operator;
    }
    const QString& getCondition11Operator() const {
        return m_Condition11Operator;
    }
    const QString& getCondition12Operator() const {
        return m_Condition12Operator;
    }
    const QString& getCondition1Value() const {
        return m_Condition1Value;
    }
    const QString& getCondition2Value() const {
        return m_Condition2Value;
    }
    const QString& getCondition3Value() const {
        return m_Condition3Value;
    }
    const QString& getCondition4Value() const {
        return m_Condition4Value;
    }
    const QString& getCondition5Value() const {
        return m_Condition5Value;
    }
    const QString& getCondition6Value() const {
        return m_Condition6Value;
    }
    const QString& getCondition7Value() const {
        return m_Condition7Value;
    }
    const QString& getCondition8Value() const {
        return m_Condition8Value;
    }
    const QString& getCondition9Value() const {
        return m_Condition9Value;
    }
    const QString& getCondition10Value() const {
        return m_Condition10Value;
    }
    const QString& getCondition11Value() const {
        return m_Condition11Value;
    }
    const QString& getCondition12Value() const {
        return m_Condition12Value;
    }
    const QString& getCondition1Combiner() const {
        return m_Condition1Combiner;
    }
    const QString& getCondition2Combiner() const {
        return m_Condition2Combiner;
    }
    const QString& getCondition3Combiner() const {
        return m_Condition3Combiner;
    }
    const QString& getCondition4Combiner() const {
        return m_Condition4Combiner;
    }
    const QString& getCondition5Combiner() const {
        return m_Condition5Combiner;
    }
    const QString& getCondition6Combiner() const {
        return m_Condition6Combiner;
    }
    const QString& getCondition7Combiner() const {
        return m_Condition7Combiner;
    }
    const QString& getCondition8Combiner() const {
        return m_Condition8Combiner;
    }
    const QString& getCondition9Combiner() const {
        return m_Condition9Combiner;
    }
    const QString& getCondition10Combiner() const {
        return m_Condition10Combiner;
    }
    const QString& getCondition11Combiner() const {
        return m_Condition11Combiner;
    }
    const QString& getCondition12Combiner() const {
        return m_Condition12Combiner;
    }

    void setCondition1Field(QString Condition1Field) {
        // DEBUG_ASSERT(!Condition1Field.isEmpty());
        m_Condition1Field = std::move(Condition1Field);
    }
    void setCondition2Field(QString Condition2Field) {
        // DEBUG_ASSERT(!Condition2Field.isEmpty());
        m_Condition2Field = std::move(Condition2Field);
    }
    void setCondition3Field(QString Condition3Field) {
        // DEBUG_ASSERT(!Condition3Field.isEmpty());
        m_Condition3Field = std::move(Condition3Field);
    }
    void setCondition4Field(QString Condition4Field) {
        //        // DEBUG_ASSERT(!Condition4Field.isEmpty());
        m_Condition4Field = std::move(Condition4Field);
    }
    void setCondition5Field(QString Condition5Field) {
        // DEBUG_ASSERT(!Condition5Field.isEmpty());
        m_Condition5Field = std::move(Condition5Field);
    }
    void setCondition6Field(QString Condition6Field) {
        // DEBUG_ASSERT(!Condition6Field.isEmpty());
        m_Condition6Field = std::move(Condition6Field);
    }
    void setCondition7Field(QString Condition7Field) {
        // DEBUG_ASSERT(!Condition7Field.isEmpty());
        m_Condition7Field = std::move(Condition7Field);
    }
    void setCondition8Field(QString Condition8Field) {
        // DEBUG_ASSERT(!Condition8Field.isEmpty());
        m_Condition8Field = std::move(Condition8Field);
    }
    void setCondition9Field(QString Condition9Field) {
        // DEBUG_ASSERT(!Condition9Field.isEmpty());
        m_Condition9Field = std::move(Condition9Field);
    }
    void setCondition10Field(QString Condition10Field) {
        // DEBUG_ASSERT(!Condition10Field.isEmpty());
        m_Condition10Field = std::move(Condition10Field);
    }
    void setCondition11Field(QString Condition11Field) {
        // DEBUG_ASSERT(!Condition11Field.isEmpty());
        m_Condition11Field = std::move(Condition11Field);
    }
    void setCondition12Field(QString Condition12Field) {
        // DEBUG_ASSERT(!Condition12Field.isEmpty());
        m_Condition12Field = std::move(Condition12Field);
    }
    void setCondition1Operator(QString Condition1Operator) {
        // DEBUG_ASSERT(!Condition1Operator.isEmpty());
        m_Condition1Operator = std::move(Condition1Operator);
    }
    void setCondition2Operator(QString Condition2Operator) {
        // DEBUG_ASSERT(!Condition2Operator.isEmpty());
        m_Condition2Operator = std::move(Condition2Operator);
    }
    void setCondition3Operator(QString Condition3Operator) {
        // DEBUG_ASSERT(!Condition3Operator.isEmpty());
        m_Condition3Operator = std::move(Condition3Operator);
    }
    void setCondition4Operator(QString Condition4Operator) {
        // DEBUG_ASSERT(!Condition4Operator.isEmpty());
        m_Condition4Operator = std::move(Condition4Operator);
    }
    void setCondition5Operator(QString Condition5Operator) {
        // DEBUG_ASSERT(!Condition5Operator.isEmpty());
        m_Condition5Operator = std::move(Condition5Operator);
    }
    void setCondition6Operator(QString Condition6Operator) {
        // DEBUG_ASSERT(!Condition6Operator.isEmpty());
        m_Condition6Operator = std::move(Condition6Operator);
    }
    void setCondition7Operator(QString Condition7Operator) {
        // DEBUG_ASSERT(!Condition7Operator.isEmpty());
        m_Condition7Operator = std::move(Condition7Operator);
    }
    void setCondition8Operator(QString Condition8Operator) {
        // DEBUG_ASSERT(!Condition8Operator.isEmpty());
        m_Condition8Operator = std::move(Condition8Operator);
    }
    void setCondition9Operator(QString Condition9Operator) {
        // DEBUG_ASSERT(!Condition9Operator.isEmpty());
        m_Condition9Operator = std::move(Condition9Operator);
    }
    void setCondition10Operator(QString Condition10Operator) {
        // DEBUG_ASSERT(!Condition10Operator.isEmpty());
        m_Condition10Operator = std::move(Condition10Operator);
    }
    void setCondition11Operator(QString Condition11Operator) {
        // DEBUG_ASSERT(!Condition11Operator.isEmpty());
        m_Condition11Operator = std::move(Condition11Operator);
    }
    void setCondition12Operator(QString Condition12Operator) {
        // DEBUG_ASSERT(!Condition12Operator.isEmpty());
        m_Condition12Operator = std::move(Condition12Operator);
    }
    void setCondition1Value(QString Condition1Value) {
        // DEBUG_ASSERT(!Condition1Value.isEmpty());
        m_Condition1Value = std::move(Condition1Value);
    }
    void setCondition2Value(QString Condition2Value) {
        // DEBUG_ASSERT(!Condition2Value.isEmpty());
        m_Condition2Value = std::move(Condition2Value);
    }
    void setCondition3Value(QString Condition3Value) {
        // DEBUG_ASSERT(!Condition3Value.isEmpty());
        m_Condition3Value = std::move(Condition3Value);
    }
    void setCondition4Value(QString Condition4Value) {
        // DEBUG_ASSERT(!Condition4Value.isEmpty());
        m_Condition4Value = std::move(Condition4Value);
    }
    void setCondition5Value(QString Condition5Value) {
        // DEBUG_ASSERT(!Condition5Value.isEmpty());
        m_Condition5Value = std::move(Condition5Value);
    }
    void setCondition6Value(QString Condition6Value) {
        // DEBUG_ASSERT(!Condition6Value.isEmpty());
        m_Condition6Value = std::move(Condition6Value);
    }
    void setCondition7Value(QString Condition7Value) {
        // DEBUG_ASSERT(!Condition7Value.isEmpty());
        m_Condition7Value = std::move(Condition7Value);
    }
    void setCondition8Value(QString Condition8Value) {
        // DEBUG_ASSERT(!Condition8Value.isEmpty());
        m_Condition8Value = std::move(Condition8Value);
    }
    void setCondition9Value(QString Condition9Value) {
        // DEBUG_ASSERT(!Condition9Value.isEmpty());
        m_Condition9Value = std::move(Condition9Value);
    }
    void setCondition10Value(QString Condition10Value) {
        // DEBUG_ASSERT(!Condition10Value.isEmpty());
        m_Condition10Value = std::move(Condition10Value);
    }
    void setCondition11Value(QString Condition11Value) {
        // DEBUG_ASSERT(!Condition11Value.isEmpty());
        m_Condition11Value = std::move(Condition11Value);
    }
    void setCondition12Value(QString Condition12Value) {
        // DEBUG_ASSERT(!Condition12Value.isEmpty());
        m_Condition12Value = std::move(Condition12Value);
    }
    void setCondition1Combiner(QString Condition1Combiner) {
        // DEBUG_ASSERT(!Condition1Combiner.isEmpty());
        m_Condition1Combiner = std::move(Condition1Combiner);
    }
    void setCondition2Combiner(QString Condition2Combiner) {
        // DEBUG_ASSERT(!Condition2Combiner.isEmpty());
        m_Condition2Combiner = std::move(Condition2Combiner);
    }
    void setCondition3Combiner(QString Condition3Combiner) {
        // DEBUG_ASSERT(!Condition3Combiner.isEmpty());
        m_Condition3Combiner = std::move(Condition3Combiner);
    }
    void setCondition4Combiner(QString Condition4Combiner) {
        // DEBUG_ASSERT(!Condition4Combiner.isEmpty());
        m_Condition4Combiner = std::move(Condition4Combiner);
    }
    void setCondition5Combiner(QString Condition5Combiner) {
        // DEBUG_ASSERT(!Condition5Combiner.isEmpty());
        m_Condition5Combiner = std::move(Condition5Combiner);
    }
    void setCondition6Combiner(QString Condition6Combiner) {
        // DEBUG_ASSERT(!Condition6Combiner.isEmpty());
        m_Condition6Combiner = std::move(Condition6Combiner);
    }
    void setCondition7Combiner(QString Condition7Combiner) {
        // DEBUG_ASSERT(!Condition7Combiner.isEmpty());
        m_Condition7Combiner = std::move(Condition7Combiner);
    }
    void setCondition8Combiner(QString Condition8Combiner) {
        // DEBUG_ASSERT(!Condition8Combiner.isEmpty());
        m_Condition8Combiner = std::move(Condition8Combiner);
    }
    void setCondition9Combiner(QString Condition9Combiner) {
        // DEBUG_ASSERT(!Condition9Combiner.isEmpty());
        m_Condition9Combiner = std::move(Condition9Combiner);
    }
    void setCondition10Combiner(QString Condition10Combiner) {
        // DEBUG_ASSERT(!Condition10Combiner.isEmpty());
        m_Condition10Combiner = std::move(Condition10Combiner);
    }
    void setCondition11Combiner(QString Condition11Combiner) {
        // DEBUG_ASSERT(!Condition11Combiner.isEmpty());
        m_Condition11Combiner = std::move(Condition11Combiner);
    }
    void setCondition12Combiner(QString Condition12Combiner) {
        // DEBUG_ASSERT(!Condition12Combiner.isEmpty());
        m_Condition12Combiner = std::move(Condition12Combiner);
    }

  protected:
    DbNamedEntity() = default;
    explicit DbNamedEntity(T id)
        : DbEntity<T>(std::forward<T>(id)) {
    }

  private:
    QString m_name;
    QString m_searchInput;
    QString m_searchSql;

    QString m_Condition1Field;
    QString m_Condition2Field;
    QString m_Condition3Field;
    QString m_Condition4Field;
    QString m_Condition5Field;
    QString m_Condition6Field;
    QString m_Condition7Field;
    QString m_Condition8Field;
    QString m_Condition9Field;
    QString m_Condition10Field;
    QString m_Condition11Field;
    QString m_Condition12Field;

    QString m_Condition1Operator;
    QString m_Condition2Operator;
    QString m_Condition3Operator;
    QString m_Condition4Operator;
    QString m_Condition5Operator;
    QString m_Condition6Operator;
    QString m_Condition7Operator;
    QString m_Condition8Operator;
    QString m_Condition9Operator;
    QString m_Condition10Operator;
    QString m_Condition11Operator;
    QString m_Condition12Operator;

    QString m_Condition1Value;
    QString m_Condition2Value;
    QString m_Condition3Value;
    QString m_Condition4Value;
    QString m_Condition5Value;
    QString m_Condition6Value;
    QString m_Condition7Value;
    QString m_Condition8Value;
    QString m_Condition9Value;
    QString m_Condition10Value;
    QString m_Condition11Value;
    QString m_Condition12Value;

    QString m_Condition1Combiner;
    QString m_Condition2Combiner;
    QString m_Condition3Combiner;
    QString m_Condition4Combiner;
    QString m_Condition5Combiner;
    QString m_Condition6Combiner;
    QString m_Condition7Combiner;
    QString m_Condition8Combiner;
    QString m_Condition9Combiner;
    QString m_Condition10Combiner;
    QString m_Condition11Combiner;
    QString m_Condition12Combiner;
};

template<typename T>
QDebug operator<<(QDebug debug, const DbNamedEntity<T>& entity) {
    return debug << QString("%1 '%2'").arg(entity.getId().toString(), entity.getName());
}
