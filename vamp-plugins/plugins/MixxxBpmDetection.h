#ifndef _MIXXXBPMDETECTION_H_
#define _MIXXXBPMDETECTION_H_

#include <vamp-sdk/Plugin.h>
#include <BPMDetect.h>

class MixxxBpmDetection : public Vamp::Plugin {
  public:
    MixxxBpmDetection(float inputSampleRate);
    ~MixxxBpmDetection() override;

    std::string getIdentifier() const override;
    std::string getName() const override;
    std::string getDescription() const override;
    std::string getMaker() const override;
    int getPluginVersion() const override;
    std::string getCopyright() const override;

    InputDomain getInputDomain() const override;
    size_t getPreferredBlockSize() const override;
    size_t getPreferredStepSize() const override;
    size_t getMinChannelCount() const override;
    size_t getMaxChannelCount() const override;

    ParameterList getParameterDescriptors() const override;
    float getParameter(std::string identifier) const override;
    void setParameter(std::string identifier, float value) override;

    ProgramList getPrograms() const override;
    std::string getCurrentProgram() const override;
    void selectProgram(std::string name) override;

    OutputList getOutputDescriptors() const override;

    bool initialise(size_t channels, size_t stepSize, size_t blockSize) override;
    void reset() override;

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

  protected:
    // plugin-specific data and methods go here
    float correctBPM(float BPM, float min, float max, bool aboveRange);

    soundtouch::BPMDetect *m_pDetector;
    int m_iSampleRate,
        m_iBlockSize;
    float m_fPhase, m_fNumCycles, m_fMinBpm, m_fMaxBpm;
    bool m_bProcessEntireSong, m_bAllowAboveRange;
};

#endif
