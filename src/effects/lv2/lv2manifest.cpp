#include "effects/lv2/lv2manifest.h"

#include "effects/effectmanifestparameter.h"
#include "util/fpclassify.h"

LV2Manifest::LV2Manifest(const LilvPlugin* plug,
                         QHash<QString, LilvNode*>& properties)
        : m_pEffectManifest(new EffectManifest()),
          m_status(AVAILABLE) {

    m_pLV2plugin = plug;

    // Get and set the ID
    const LilvNode* id = lilv_plugin_get_uri(m_pLV2plugin);
    m_pEffectManifest->setId(lilv_node_as_string(id));

    // Get and set the name
    LilvNode* info = lilv_plugin_get_name(m_pLV2plugin);
    m_pEffectManifest->setName(lilv_node_as_string(info));
    lilv_node_free(info);

    // Get and set the author
    info = lilv_plugin_get_author_name(m_pLV2plugin);
    m_pEffectManifest->setAuthor(lilv_node_as_string(info));
    lilv_node_free(info);

    int numPorts = lilv_plugin_get_num_ports(plug);
    m_minimum = new float[numPorts];
    m_maximum = new float[numPorts];
    m_default = new float[numPorts];
    lilv_plugin_get_port_ranges_float(m_pLV2plugin, m_minimum, m_maximum,
                                      m_default);

    // Counters to determine the type of the plug in
    int inputPorts = 0;
    int outputPorts = 0;

    for (int i = 0; i < numPorts; i++) {
        const LilvPort *port = lilv_plugin_get_port_by_index(plug, i);

        if (lilv_port_is_a(m_pLV2plugin, port, properties["audio_port"])) {
            if (lilv_port_is_a(m_pLV2plugin, port, properties["input_port"])) {
                audioPortIndices.append(i);
                inputPorts++;
            } else if (lilv_port_is_a(m_pLV2plugin, port, properties["output_port"])) {
                audioPortIndices.append(i);
                outputPorts++;
            }
        }

        if (lilv_port_is_a(m_pLV2plugin, port, properties["control_port"])
                && !lilv_port_has_property(m_pLV2plugin, port, properties["enumeration_port"])
                && !lilv_port_has_property(m_pLV2plugin, port, properties["button_port"])) {
            controlPortIndices.append(i);
            EffectManifestParameterPointer param = m_pEffectManifest->addParameter();

            // Get and set the parameter name
            info = lilv_port_get_name(m_pLV2plugin, port);
            QString paramName = lilv_node_as_string(info);
            param->setName(paramName);
            lilv_node_free(info);

            const LilvNode* node = lilv_port_get_symbol(m_pLV2plugin, port);
            QString symbol = lilv_node_as_string(node);
            param->setId(symbol);
            // node must not be freed here, it is owned by port

            param->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
            param->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
            param->setDefault(m_default[i]);
            param->setMinimum(m_minimum[i]);
            param->setMaximum(m_maximum[i]);

            // Set the appropriate Hints
            if (lilv_port_has_property(m_pLV2plugin, port, properties["button_port"])) {
                param->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
            } else if (lilv_port_has_property(m_pLV2plugin, port, properties["enumeration_port"])) {
                buildEnumerationOptions(port, param);
                param->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
            } else if (lilv_port_has_property(m_pLV2plugin, port, properties["integer_port"])) {
                param->setControlHint(EffectManifestParameter::ControlHint::KNOB_STEPPING);
            } else {
                 param->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
            }
        }
    }

    // Hack for putting enum parameters to the end of controlportindices
    for (int i = 0; i < numPorts; i++) {
        const LilvPort *port = lilv_plugin_get_port_by_index(plug, i);

        if (lilv_port_is_a(m_pLV2plugin, port, properties["control_port"]) &&
                (lilv_port_has_property(m_pLV2plugin, port, properties["enumeration_port"]) ||
                 lilv_port_has_property(m_pLV2plugin, port, properties["button_port"]))) {
            controlPortIndices.append(i);
            EffectManifestParameterPointer param = m_pEffectManifest->addParameter();

            // Get and set the parameter name
            info = lilv_port_get_name(m_pLV2plugin, port);
            QString paramName = lilv_node_as_string(info);
            param->setName(paramName);
            lilv_node_free(info);

            const LilvNode* node = lilv_port_get_symbol(m_pLV2plugin, port);
            QString symbol = lilv_node_as_string(node);
            param->setId(symbol);
            // info must not be freed here, it is owned by port

            param->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
            param->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
            param->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
            if (lilv_port_has_property(m_pLV2plugin, port, properties["enumeration_port"])) {
                buildEnumerationOptions(port, param);
            } else {
                param->appendStep(qMakePair(QString("Inactive"), 0.0));
                param->appendStep(qMakePair(QString("Active"), 1.0));
            }

            // Some plugins don't specify minimum, maximum and default values
            // In this case set the minimum and default values to 0 and
            // the maximum to the number of scale points
            if (util_isnan(m_default[i])) {
                param->setDefault(0);
            } else {
                param->setDefault(m_default[i]);
            }

            if (util_isnan(m_minimum[i])) {
                param->setMinimum(0);
            } else {
                param->setMinimum(m_minimum[i]);
            }

            if (util_isnan(m_maximum[i])) {
                param->setMaximum(param->getSteps().size() - 1);
            } else {
                param->setMaximum(m_maximum[i]);
            }
        }
    }

    // We only support the case when the input and output samples are stereo
    if (inputPorts != 2 || outputPorts != 2) {
        m_status = IO_NOT_STEREO;
    }

    // We don't support any features
    LilvNodes* features = lilv_plugin_get_required_features(m_pLV2plugin);
    if (lilv_nodes_size(features) > 0) {
        m_status = HAS_REQUIRED_FEATURES;
    }
    lilv_nodes_free(features);
}

LV2Manifest::~LV2Manifest() {
    delete[] m_minimum;
    delete[] m_maximum;
    delete[] m_default;
}

EffectManifestPointer LV2Manifest::getEffectManifest() const {
    return m_pEffectManifest;
}

QList<int> LV2Manifest::getAudioPortIndices() {
    return audioPortIndices;
}

QList<int> LV2Manifest::getControlPortIndices() {
    return controlPortIndices;
}

const LilvPlugin* LV2Manifest::getPlugin() {
    return m_pLV2plugin;
}

LV2Manifest::Status LV2Manifest::getStatus() {
    return m_status;
}

bool LV2Manifest::isValid() {
    return m_status == AVAILABLE;
}

void LV2Manifest::buildEnumerationOptions(const LilvPort* port,
                                          EffectManifestParameterPointer param) {
    LilvScalePoints* options = lilv_port_get_scale_points(m_pLV2plugin, port);
    LILV_FOREACH(scale_points, iterator, options) {
        const LilvScalePoint* option = lilv_scale_points_get(options, iterator);
        const LilvNode* description = lilv_scale_point_get_label(option);
        const LilvNode* value = lilv_scale_point_get_value(option);
        QString strDescription(lilv_node_as_string(description));
        param->appendStep(qMakePair(strDescription,
                static_cast<double>(lilv_node_as_float(value))));
    }

    if (options != nullptr) {
        lilv_scale_points_free(options);
    }
}
