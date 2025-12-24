#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"

#include "controllers/hid/legacyhidcontrollermapping.h"

bool LegacyHidControllerMappingFileHandler::save(const LegacyHidControllerMapping& mapping,
        const QString& fileName) const {
    QDomDocument doc = buildRootWithScripts(mapping);
    return writeDocument(doc, fileName);
}

std::shared_ptr<LegacyControllerMapping>
LegacyHidControllerMappingFileHandler::load(const QDomElement& root,
        const QString& filePath,
        const QDir& systemMappingsPath) {
    if (root.isNull()) {
        return nullptr;
    }

    QDomElement controller = getControllerNode(root);
    if (controller.isNull()) {
        return nullptr;
    }

    auto pMapping = std::make_shared<LegacyHidControllerMapping>();
    pMapping->setFilePath(filePath);
    parseMappingInfo(root, pMapping);
    parseMappingSettings(root, pMapping.get());
    addScriptFilesToMapping(controller, pMapping, systemMappingsPath);
    return pMapping;
}
