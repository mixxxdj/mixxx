#pragma once

// Type-safe wrapper with a default constructor that encapsulates
// field indices of QSqlRecord.
class DbFieldIndex {
public:
    static const int INVALID_INDEX = -1;

    // Implicit conversion from int
    DbFieldIndex(int index = INVALID_INDEX)
        : m_index(index) {
    }

    // Implicit conversion to int
    operator int() const {
        return m_index;
    }

    bool isValid() const {
        return m_index >= 0;
    }

private:
    int m_index;
};

Q_DECLARE_METATYPE(DbFieldIndex)
