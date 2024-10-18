#include "effects/backends/lv2/lv2manifest.h"

#include "effects/backends/effectmanifestparameter.h"
#include "util/fpclassify.h"

namespace {
constexpr bool lv2ParamDebug = true;
} // namespace

LV2Manifest::LV2Manifest(LilvWorld* world,
        const LilvPlugin* plug,
        QHash<QString, LilvNode*>& properties)
        : EffectManifest(),
          m_minimum(lilv_plugin_get_num_ports(plug)),
          m_maximum(lilv_plugin_get_num_ports(plug)),
          m_default(lilv_plugin_get_num_ports(plug)),
          m_status(AVAILABLE) {
    m_pLV2plugin = plug;

    // Get and set the ID
    const LilvNode* id = lilv_plugin_get_uri(m_pLV2plugin);
    setId(lilv_node_as_string(id));

    // Get and set the name
    LilvNode* info = lilv_plugin_get_name(m_pLV2plugin);
    setName(lilv_node_as_string(info));
    lilv_node_free(info);

    // Get and set the author
    info = lilv_plugin_get_author_name(m_pLV2plugin);
    setAuthor(lilv_node_as_string(info));
    lilv_node_free(info);

    int numPorts = lilv_plugin_get_num_ports(plug);
    lilv_plugin_get_port_ranges_float(
            m_pLV2plugin, m_minimum.data(), m_maximum.data(), m_default.data());

    // Counters to determine the type of the plug in
    int inputPorts = 0;
    int outputPorts = 0;

    for (int i = 0; i < numPorts; i++) {
        const LilvPort* port = lilv_plugin_get_port_by_index(plug, i);

        if (lilv_port_is_a(m_pLV2plugin, port, properties["audio_port"])) {
            if (lilv_port_is_a(m_pLV2plugin, port, properties["input_port"])) {
                audioPortIndices.append(i);
                inputPorts++;
            } else if (lilv_port_is_a(m_pLV2plugin, port, properties["output_port"])) {
                audioPortIndices.append(i);
                outputPorts++;
            }
        }

        // Does this detect a knob/slider parameter?
        if (lilv_port_is_a(m_pLV2plugin, port, properties["control_port"]) &&
                !lilv_port_has_property(
                        m_pLV2plugin, port, properties["enumeration_port"]) &&
                !lilv_port_has_property(
                        m_pLV2plugin, port, properties["button_port"])) {
            if (util_isnan(m_minimum[i]) || util_isnan(m_maximum[i])) {
                continue;
            }
            controlPortIndices.append(i);
            EffectManifestParameterPointer param = addParameter();

            // Get and set the parameter name
            info = lilv_port_get_name(m_pLV2plugin, port);
            param->setName(lilv_node_as_string(info));
            lilv_node_free(info);

            const LilvNode* node = lilv_port_get_symbol(m_pLV2plugin, port);
            param->setId(lilv_node_as_string(node));
            // node must not be freed here, it is owned by port

            EffectManifestParameter::UnitsHint unitsHint =
                    EffectManifestParameter::UnitsHint::Unknown;
            LilvNodes* units = lilv_port_get_value(m_pLV2plugin, port, properties["unit"]);
            if (lilv_nodes_size(units) > 0) {
                LilvNode* unit = lilv_nodes_get_first(units);
                // If this is a unit symbol URI, e.g. http://lv2plug.in/ns/extensions/units#ms
                if (lilv_node_is_uri(unit)) {
                    QString unitStr = lilv_node_as_uri(unit);
                    // that starts with the 'units' prefix, isolate the identifier
                    if (unitStr.startsWith(lilv_node_as_string(properties["unit_prefix"]))) {
                        unitsHint = param->lv2UnitToUnitsHint(
                                unitStr.remove(lilv_node_as_string(properties["unit_prefix"])));
                    }
                } else { // Try to extract the custom unit symbol string
                    LilvNode* customUnit =
                            lilv_world_get(world, unit, properties["unit_symbol"], nullptr);
                    if (customUnit) {
                        // Accepted custom units needs to be 'whitelisted' in
                        // EffectManifestParameter::lv2UnitToUnitsHint and added to
                        // EffectManifestParameter::unitsHintStringHash
                        unitsHint = param->lv2UnitToUnitsHint(
                                lilv_node_as_string(customUnit));
                        if (lv2ParamDebug &&
                                unitsHint == EffectManifestParameter::UnitsHint::Unknown &&
                                !param->ignoreCustomUnit(lilv_node_as_string(customUnit))) {
                            qDebug().nospace()
                                    << "LV2Manifest: plugin \""
                                    << lilv_node_as_string(lilv_plugin_get_name(
                                               m_pLV2plugin))
                                    << "\" has custom unit \""
                                    << lilv_node_as_string(customUnit)
                                    << "\" for parameter " << param->name();
                        }
                    }
                }
            }
            param->setUnitsHint(unitsHint);
            lilv_nodes_free(units);

            if (util_isnan(m_default[i]) || m_default[i] < m_minimum[i] ||
                    m_default[i] > m_maximum[i]) {
                m_default[i] = m_minimum[i];
            }
            param->setRange(m_minimum[i], m_default[i], m_maximum[i]);

            if (lilv_port_has_property(m_pLV2plugin, port, properties["integer_port"])) {
                param->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
            } else {
                param->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
            }
        }
    }

    // Hack for putting enum parameters to the end of controlportindices
    for (int i = 0; i < numPorts; i++) {
        const LilvPort* port = lilv_plugin_get_port_by_index(plug, i);

        if (lilv_port_is_a(m_pLV2plugin, port, properties["control_port"]) &&
                (lilv_port_has_property(m_pLV2plugin, port, properties["enumeration_port"]) ||
                        lilv_port_has_property(m_pLV2plugin, port, properties["button_port"]))) {
            controlPortIndices.append(i);
            EffectManifestParameterPointer param = addParameter();

            // Get and set the parameter name
            info = lilv_port_get_name(m_pLV2plugin, port);
            param->setName(lilv_node_as_string(info));
            lilv_node_free(info);

            const LilvNode* node = lilv_port_get_symbol(m_pLV2plugin, port);
            param->setId(lilv_node_as_string(node));
            // info must not be freed here, it is owned by port

            param->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
            param->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
            if (lilv_port_has_property(m_pLV2plugin, port, properties["enumeration_port"])) {
                buildEnumerationOptions(port, param);
            } else {
                param->appendStep(qMakePair(QString("Inactive"), 0.0));
                param->appendStep(qMakePair(QString("Active"), 1.0));
            }

            // Some plugins don't specify minimum, maximum and default values
            // In this case set the minimum and default values to 0 and
            // the maximum to the number of scale points
            double minimum = 0;
            double defaultValue = 0;
            double maximum = param->getSteps().size() - 1;

            if (!util_isnan(m_minimum[i])) {
                minimum = m_minimum[i];
            }
            if (!util_isnan(m_default[i])) {
                defaultValue = m_default[i];
            }
            if (!util_isnan(m_maximum[i])) {
                maximum = m_maximum[i];
            }

            param->setRange(minimum, defaultValue, maximum);
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

        param->appendStep(qMakePair(QString(lilv_node_as_string(description)),
                static_cast<double>(lilv_node_as_float(value))));
    }

    if (options != nullptr) {
        lilv_scale_points_free(options);
    }
}
