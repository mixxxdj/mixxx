#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"

bool LegacyHidControllerMappingFileHandler::save(const LegacyHidControllerMapping& mapping,
        const QString& fileName) const {
    QDomDocument doc = buildRootWithScripts(mapping);
    return writeDocument(doc, fileName);
}

LegacyControllerMappingPointer LegacyHidControllerMappingFileHandler::load(const QDomElement& root,
        const QString& filePath,
        const QDir& systemMappingsPath) {
    if (root.isNull()) {
        return LegacyControllerMappingPointer();
    }

    QDomElement controller = getControllerNode(root);
    if (controller.isNull()) {
        return LegacyControllerMappingPointer();
    }

    LegacyHidControllerMapping* mapping = new LegacyHidControllerMapping();
    mapping->setFilePath(filePath);
    parseMappingInfo(root, mapping);
    addScriptFilesToMapping(controller, mapping, systemMappingsPath);
    return LegacyControllerMappingPointer(mapping);
}
