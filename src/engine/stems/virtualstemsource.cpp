#include "virtualstemsource.h"
#include "moc_virtualstemsource.cpp"

#include "virtualstemsoundsource.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("VirtualStemSource");
} // namespace

VirtualStemSource::VirtualStemSource(const StemCacheManager::StemFiles& files, int sampleRate)
    : QObject(nullptr), m_sampleRate(sampleRate) {
    m_stemPaths[0] = files.vocals;
    m_stemPaths[1] = files.drums;
    m_stemPaths[2] = files.bass;
    m_stemPaths[3] = files.other;
}

VirtualStemSource::~VirtualStemSource() = default;

bool VirtualStemSource::load() {
    if (m_loaded) return true;
    
    qint64 minFrames = -1;
    bool allOk = true;
    
    // First pass: load all stems
    for (int i = 0; i < 4; ++i) {
        if (m_stemPaths[i].isEmpty()) {
            kLogger.warning() << "Stem" << i << "path is empty";
            allOk = false;
            continue;
        }
        
        m_sources[i] = std::make_unique<mixxx::VirtualStemSoundSource>(m_stemPaths[i], m_sampleRate);
        if (!m_sources[i]->load()) {
            kLogger.warning() << "Failed to load stem" << i << ":" << m_stemPaths[i];
            allOk = false;
        } else {
            qint64 frames = m_sources[i]->getTotalFrames();
            if (minFrames == -1 || frames < minFrames) {
                minFrames = frames;
            }
            kLogger.info() << "Loaded stem" << i << "frames:" << frames;
        }
    }
    
    if (!allOk || minFrames <= 0) {
        kLogger.warning() << "Failed to load all stems";
        return false;
    }
    
    // Use minimum frame count for all stems
    m_totalFrames = minFrames;
    m_loaded = true;
    
    kLogger.info() << "Virtual stem source loaded:" << m_totalFrames << "frames @ " << m_sampleRate << "Hz";
    return true;
}

mixxx::VirtualStemSoundSource* VirtualStemSource::getStemSource(int stemIdx) const {
    if (stemIdx >= 0 && stemIdx < 4 && m_loaded) {
        return m_sources[stemIdx].get();
    }
    return nullptr;
}