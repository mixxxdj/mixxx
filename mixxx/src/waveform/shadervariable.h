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
        m_program(0),
        m_id(id),
        m_value(value) {}

    const QString& getName() const { return m_name;}
    GLint getId() const { return m_id;}
    T getValue() const { return m_value;}
    void setValue( const T& value) { m_value = value;}

    bool isValid() const { return m_id > -1;}

    bool initUniformLocation( QGLShaderProgram* program) {
        m_id = program->uniformLocation(m_name);
        m_program = program;
        return m_id != -1;
    }

    bool initAttributeLocation( QGLShaderProgram* program) {
        m_id = program->attributeLocation(m_name);
        m_program = program;
        return m_id != -1;
    }

    bool setUniformValue() const {
        if( !isValid()) return false;
        m_program->setUniformValue( m_id, m_value);
        return true;
    }

    bool setUniformValue( const T& value) {
        if( !isValid()) return false;
        m_value = value;
        m_program->setUniformValue( m_id, m_value);
        return true;
    }

    bool setAttributeValue() const {
        if( !isValid()) return false;
        m_program->setAttributeValue( m_id, m_value);
        return true;
    }

    bool setAttributeValue(const T& value) {
        if( !isValid()) return false;
        m_value = value;
        m_program->setAttributeValue( m_id, m_value);
        return true;
    }

private:
    QString m_name;
    QGLShaderProgram* m_program;
    int m_id;
    T m_value;
};

#endif // SHADERVARIABLE_H
