#include <QDomDocument>

#include "test/mixxxtest.h"
#include "skin/skincontext.h"

class SkinContextTest : public MixxxTest {
  public:
    SkinContextTest()
            : m_context(config(), "test") {
    }

    virtual ~SkinContextTest() {
    }

  protected:
    SkinContext m_context;
};

TEST_F(SkinContextTest, TestVariable) {
    // Basic check that variable lookup works.
    m_context.setVariable("test", "asdf");
    EXPECT_QSTRING_EQ("asdf", m_context.variable("test"));
}

TEST_F(SkinContextTest, UpdateVariables) {
    // Verify that updateVariables works on all 3 types of <SetVariable> nodes.
    m_context.setVariable("test", "asdf");
    m_context.setVariable("test2", "1234");
    m_context.setVariable("test3", "foo");
    m_context.setVariable("test4", "bar");

    QDomDocument doc;
    QDomElement tmpl = doc.createElement("Template");

    // Set test to 'zxcv'.
    QDomElement var1 = doc.createElement("SetVariable");
    var1.setAttribute("name", "test");
    var1.appendChild(doc.createTextNode("zxcv"));
    tmpl.appendChild(var1);

    // Set test2 to the pattern %1_%1.
    QDomElement var2 = doc.createElement("SetVariable");
    var2.setAttribute("name", "test2");
    var2.setAttribute("format", "%1_%1");
    tmpl.appendChild(var2);

    m_context.updateVariables(tmpl);

    EXPECT_QSTRING_EQ("zxcv", m_context.variable("test"));
    EXPECT_QSTRING_EQ("1234_1234", m_context.variable("test2"));
    EXPECT_QSTRING_EQ("foo", m_context.variable("test3"));
    EXPECT_QSTRING_EQ("bar", m_context.variable("test4"));
}

TEST_F(SkinContextTest, UpdateVariables_EmbeddedVariable) {
    // Check that an embedded <Variable> node inside a <SetVariable> node is
    // evaluated before assigning the variable.
    m_context.setVariable("test", "asdf");
    QDomDocument doc;
    QDomElement tmpl = doc.createElement("Template");
    QDomElement outerVar = doc.createElement("SetVariable");
    outerVar.setAttribute("name", "test2");
    outerVar.appendChild(doc.createTextNode("zxcv"));
    QDomElement innerVar = doc.createElement("Variable");
    innerVar.setAttribute("name", "test");
    outerVar.appendChild(innerVar);
    tmpl.appendChild(outerVar);

    m_context.updateVariables(tmpl);

    EXPECT_QSTRING_EQ("zxcvasdf", m_context.variable("test2"));
}

TEST_F(SkinContextTest, NoVariable) {
    // Test that converting a node to string with no variable reference works.
    QDomDocument doc;
    QDomElement test = doc.createElement("Test");
    test.appendChild(doc.createTextNode("Hello There"));
    EXPECT_QSTRING_EQ("Hello There", m_context.nodeToString(test));
}

TEST_F(SkinContextTest, VariableByName) {
    // Evaluate a variable by name.
    QDomDocument doc;
    QDomElement test = doc.createElement("Test");
    test.appendChild(doc.createTextNode("Hello "));
    QDomElement variableNode = doc.createElement("Variable");
    variableNode.setAttribute("name", "name");
    test.appendChild(variableNode);
    m_context.setVariable("name", "Mixxx");
    EXPECT_QSTRING_EQ("Hello Mixxx", m_context.nodeToString(test));
}

TEST_F(SkinContextTest, VariableWithFormat) {
    // Evaluate a variable with a format string.
    QDomDocument doc;
    QDomElement test = doc.createElement("Test");
    test.appendChild(doc.createTextNode("Hello "));
    QDomElement variableNode = doc.createElement("Variable");
    variableNode.setAttribute("name", "name");
    variableNode.setAttribute("format", "-%1-");
    test.appendChild(variableNode);
    m_context.setVariable("name", "Mixxx");
    EXPECT_QSTRING_EQ("Hello -Mixxx-", m_context.nodeToString(test));
}
