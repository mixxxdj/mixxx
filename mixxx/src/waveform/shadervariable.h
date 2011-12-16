#ifndef SHADERVARIABLE_H
#define SHADERVARIABLE_H

#include <QString>

#include <QtOpenGL/QGLShaderProgram>

template<typename T>
class ShaderVariable
{
public:
    ShaderVariable( const QString& name, GLint id = -1, T value = T()) :
        m_name(name),
        m_id(id),
        m_value(value) {}

    const QString& getName() const { return m_name;}
    GLint getId() const { return m_id;}
    T getValue() const { return m_value;}

    bool isValid() const { return m_id > -1;}

    bool initUniformLocation( QGLShaderProgram* program) {
        m_id = program->uniformLocation(m_name);
        return m_id != -1;
    }

    bool setUniformValue( QGLShaderProgram* program) const {
        if( !isValid()) return false;
        program->setUniformValue( m_id, m_value);
        return true;
    }

    bool setUniformValue( QGLShaderProgram* program, const T& value) const {
        if( !isValid()) return false;
        m_value = value;
        program->setUniformValue( m_id, m_value);
        return true;
    }

    bool initAttributeLocation( QGLShaderProgram* program) {
        m_id = program->attributeLocation(m_name);
        return m_id != -1;
    }

    bool setAttributeValue( QGLShaderProgram* program) const {
        if( !isValid()) return false;
        program->setAttributeValue( m_id, m_value);
        return true;
    }

    bool setAttributeValue( QGLShaderProgram* program, const T& value) const {
        if( !isValid()) return false;
        m_value = value;
        program->setAttributeValue( m_id, m_value);
        return true;
    }

private:
    QString m_name;
    int m_id;
    T m_value;
};

#endif // SHADERVARIABLE_H
