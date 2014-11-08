#include <gtest/gtest.h>

#include <QTestEventList>
#include <QDomDocument>

#include "test/mixxxtest.h"
#include "skin/skincontext.h"


/* 
 * inline
 * file
 * script loading
 * variable parsing
 * text value parsing
 * attribute parsing
 * first line comment
 * 
 */


class SvgParserTest : public MixxxTest {
  public:
    SvgParserTest() {
        m_svgParser(*m_context);
    }

    virtual ~SvgParserTest() {
    }

  protected:
    QString m_mocksDir;
    SkinContext m_context;
    const SvgParser m_svgParser;
};

TEST_F(SvgParserTest, ParseEmptyFile) {
    QString path = "";
    // open a file
    const QByteArray rslt = svgParser.saveToQByteArray(
            svgParser.parseSvgFile(path) );
    
    QSvgRenderer svgRenderer(rslt);
    ASSERT_TRUE(svgRenderer.isValid());
}

TEST_F(WPushButtonTest, ParseInline) {
}
