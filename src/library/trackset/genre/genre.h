#pragma once

#include "library/trackset/genre/genreid.h"
#include "util/db/dbnamedentity.h"

class Genre : public DbNamedEntity<GenreId> {
  public:
    explicit Genre(GenreId id = GenreId())
            : DbNamedEntity(id),
              m_locked(false),
              m_autoDjSource(false) {
    }
    ~Genre() override = default;

    bool isLocked() const {
        return m_locked;
    }
    void setLocked(bool locked = true) {
        m_locked = locked;
    }

    bool isAutoDjSource() const {
        return m_autoDjSource;
    }
    void setAutoDjSource(bool autoDjSource = true) {
        m_autoDjSource = autoDjSource;
    }

    const QString& nameLevel1() const {
        return m_nameLevel1;
    }
    const QString& nameLevel2() const {
        return m_nameLevel2;
    }
    const QString& nameLevel3() const {
        return m_nameLevel3;
    }
    const QString& nameLevel4() const {
        return m_nameLevel4;
    }
    const QString& nameLevel5() const {
        return m_nameLevel5;
    }
    const QString& displayGroup() const {
        return m_displayGroup;
    }
    bool isVisible() const {
        return m_isVisible;
    }
    bool isModelDefined() const {
        return m_isModelDefined;
    }
    int displayOrder() const {
        return m_displayOrder;
    }
    int count() const {
        return m_count;
    }
    int show() const {
        return m_show;
    }
    void setNameLevel1(const QString& name) {
        m_nameLevel1 = name;
    }
    void setNameLevel2(const QString& name) {
        m_nameLevel2 = name;
    }
    void setNameLevel3(const QString& name) {
        m_nameLevel3 = name;
    }
    void setNameLevel4(const QString& name) {
        m_nameLevel4 = name;
    }
    void setNameLevel5(const QString& name) {
        m_nameLevel5 = name;
    }
    void setDisplayGroup(const QString& group) {
        m_displayGroup = group;
    }
    void setVisible(bool visible) {
        m_isVisible = visible;
    }
    void setModelDefined(bool userDefined) {
        m_isModelDefined = userDefined;
    }
    void setDisplayOrder(int order) {
        m_displayOrder = order;
    }
    void setCount(int count) {
        m_count = count;
    }
    void setShow(int show) {
        m_show = show;
    }

    const QString& getNameLevel1() const {
        return m_nameLevel1;
    }
    const QString& getNameLevel2() const {
        return m_nameLevel2;
    }
    const QString& getNameLevel3() const {
        return m_nameLevel3;
    }
    const QString& getNameLevel4() const {
        return m_nameLevel4;
    }
    const QString& getNameLevel5() const {
        return m_nameLevel5;
    }
    const QString& getDisplayGroup() const {
        return m_displayGroup;
    }
    int getDisplayOrder() const {
        return m_displayOrder;
    }
    int getCount() const {
        return m_count;
    }
    int getShow() const {
        return m_show;
    }

  private:
    bool m_locked;
    bool m_autoDjSource;
    QString m_nameLevel1;
    QString m_nameLevel2;
    QString m_nameLevel3;
    QString m_nameLevel4;
    QString m_nameLevel5;
    QString m_displayGroup;
    bool m_isVisible = true;
    bool m_isModelDefined = false;
    int m_displayOrder;
    int m_count;
    int m_show;
};
