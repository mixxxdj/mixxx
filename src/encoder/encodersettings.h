#pragma once

#include <QList>
#include <QObject>
#include "util/memory.h"

/// Encoder settings interface for encoders
class EncoderSettings {
    public:
        class OptionsGroup {
            public:
              OptionsGroup(const QString& gName,
                      const QString& gCode,
                      const QList<QString>& conNames)
                      : groupName(gName),
                        groupCode(gCode),
                        controlNames(conNames) {
              }
            QString groupName;
            QString groupCode;
            QList<QString> controlNames;
        };
        enum class ChannelMode {
            AUTOMATIC=0,
            MONO=1,
            STEREO=2
        };

    virtual ~EncoderSettings() = default;

    // Returns the list of quality values supported, to assign them to the slider
    virtual QList<int> getQualityValues() const { return QList<int>(); }
    virtual int getQuality() const { return 0; }
    virtual int getQualityIndex() const { return 0; }
    // Returns the list of compression values supported, to assign them to the slider
    virtual QList<int> getCompressionValues() const { return QList<int>(); }
    // Sets the compression level
    virtual void setCompression(int compression) {  Q_UNUSED(compression); }
    virtual int getCompression() const { return 0; }
    // Returns the list of radio options to show to the user
    virtual QList<OptionsGroup> getOptionGroups() const { return QList<OptionsGroup>(); }
    // Selects the option by its index. If it is a single-element option, 
    // index 0 means disabled and 1 enabled.
    // Return the selected option of the group. If it is a single-element option, 
    // 0 means disabled and 1 enabled.
    virtual int getSelectedOption(const QString& groupCode) const {
        Q_UNUSED(groupCode);
        return 0;
    }

    virtual ChannelMode getChannelMode() const { return ChannelMode::AUTOMATIC; }

    // Returns the format subtype of this encoder settings.
    virtual QString getFormat() const = 0;
};

typedef std::shared_ptr<EncoderSettings> EncoderSettingsPointer;
