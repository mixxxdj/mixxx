#include "rendergraph/materialshader.h"

#include <QFile>
#ifdef USE_QSHADER_FOR_GL
#include <rhi/qshader.h>
#endif

using namespace rendergraph;

namespace {
#ifdef USE_QSHADER_FOR_GL
QString resource(const QString& filename) {
    return QStringLiteral(":/shaders/rendergraph/%1.qsb").arg(filename);
}

QByteArray loadShaderCodeFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODeviceBase::ReadOnly)) {
        qWarning() << "Failed to open the shader file:" << path;
        return QByteArray();
    }
    QShader qsbShader = QShader::fromSerialized(file.readAll());
    QShaderKey key(QShader::GlslShader, 120);
    return qsbShader.shader(key).shader();
}
#else
QString resource(const QString& filename) {
    return QStringLiteral(":/shaders/rendergraph/%1.gl").arg(filename);
}

QByteArray loadShaderCodeFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODeviceBase::ReadOnly)) {
        qWarning() << "Failed to open shader file:" << path;
        return QByteArray();
    }
    return file.readAll();
}
#endif
} // namespace

MaterialShader::MaterialShader(const char* vertexShaderFilename,
        const char* fragmentShaderFilename,
        const UniformSet& uniformSet,
        const AttributeSet& attributeSet) {
    const QString vertexShaderFileFullPath = resource(vertexShaderFilename);
    const QString fragmentShaderFileFullPath = resource(fragmentShaderFilename);

    addShaderFromSourceCode(QOpenGLShader::Vertex,
            loadShaderCodeFromFile(vertexShaderFileFullPath));
    addShaderFromSourceCode(QOpenGLShader::Fragment,
            loadShaderCodeFromFile(fragmentShaderFileFullPath));

    link();

    for (const auto& attribute : attributeSet.attributes()) {
        int location = QOpenGLShaderProgram::attributeLocation(attribute.m_name);
        m_attributeLocations.push_back(location);
    }
    for (const auto& uniform : uniformSet.uniforms()) {
        int location = QOpenGLShaderProgram::uniformLocation(uniform.m_name);
        m_uniformLocations.push_back(location);
    }
}
