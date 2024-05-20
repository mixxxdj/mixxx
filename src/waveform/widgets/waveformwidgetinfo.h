#pragma once

#include "waveform/widgets/waveformwidgetcategory.h"
#include "waveform/widgets/waveformwidgettype.h"

class WaveformWidgetAbstract;

class WaveformWidgetInfoBase {
  public:
    const WaveformWidgetType::Type m_type;
    const QString m_name;
    const bool m_useGL;
    const bool m_useGLES;
    const bool m_useGLSL;
    const bool m_highDetail;
    const WaveformWidgetCategory m_category;

    WaveformWidgetInfoBase(WaveformWidgetType::Type type,
            QString name,
            bool useGL,
            bool useGLES,
            bool useGLSL,
            bool highDetail,
            WaveformWidgetCategory category)
            : m_type(type),
              m_name(std::move(name)),
              m_useGL(useGL),
              m_useGLES(useGLES),
              m_useGLSL(useGLSL),
              m_highDetail(highDetail),
              m_category(category) {
    }

    virtual ~WaveformWidgetInfoBase() {
    }
    virtual std::unique_ptr<WaveformWidgetAbstract> create(
            const QString& group, QWidget* parent) const = 0;
    static const WaveformWidgetInfoBase* find(WaveformWidgetType::Type type) {
        auto it = std::find_if(infos().begin(),
                infos().end(),
                [&type](const WaveformWidgetInfoBase* x) {
                    return x->m_type == type;
                });
        return it == infos().end() ? nullptr : *it;
    }

    static std::vector<WaveformWidgetInfoBase*>& infos() {
        static std::vector<WaveformWidgetInfoBase*> s_infos;
        return s_infos;
    }
};

template<class T>
class WaveformWidgetInfo : public WaveformWidgetInfoBase {
  public:
    WaveformWidgetInfo(WaveformWidgetType::Type type,
            QString name,
            bool useGL,
            bool useGLES,
            bool useGLSL,
            bool highDetail,
            WaveformWidgetCategory category)
            : WaveformWidgetInfoBase(type,
                      name,
                      useGL,
                      useGLES,
                      useGLSL,
                      highDetail,
                      category) {
        infos().push_back(this);
    }

    std::unique_ptr<WaveformWidgetAbstract> create(
            const QString& group, QWidget* parent) const override {
        return std::make_unique<T>(*this, group, parent);
    }
};
