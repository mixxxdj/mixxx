#include "controllers/scripting/colormapperjsproxy.h"

#include <gtest/gtest.h>

#include <QJSEngine>

#include "test/mixxxtest.h"

namespace {

QJSEngine* createScriptEngine() {
    QJSEngine* pEngine = new QJSEngine();
    QJSValue mapper = pEngine->newQMetaObject(&ColorMapperJSProxy::staticMetaObject);
    pEngine->globalObject().setProperty("ColorMapper", mapper);
    return pEngine;
}

} // namespace

class ColorMapperJSProxyTest : public MixxxTest {};

TEST_F(ColorMapperJSProxyTest, GetNearestColor) {
    QJSEngine* pEngine = createScriptEngine();
    QJSValue jsval = pEngine->evaluate(
            R"JavaScript(
           var mapper = new ColorMapper({
               0xC50A08: 1,
               0x32BE44: 2,
               0x42D4F4: 3,
               0xF8D200: 4,
               0x0044FF: 5,
               0xAF00CC: 6,
               0xFCA6D7: 7,
               0xF2F2FF: 8,
           });
           /* white */
           var color1 = mapper.getNearestColor(0xFFFFFF);
           if (color1.red != 0xF2 || color1.green != 0xF2 || color1.blue != 0xFF) {
               throw Error();
           };
           /* white */
           var color2 = mapper.getNearestColor(0xDCDCDC);
           if (color2.red != 0xF2 || color2.green != 0xF2 || color2.blue != 0xFF) {
               throw Error();
           };
           /* red */
           var color3 = mapper.getNearestColor(0xFF0000);
           if (color3.red != 0xC5 || color3.green != 0x0A || color3.blue != 0x08) {
               throw Error();
           };
           /* yellow */
           var color4 = mapper.getNearestColor(0x22CC22);
           if (color4.red != 0x32 || color4.green != 0xBE || color4.blue != 0x44) {
               throw Error();
           }
           )JavaScript");
    EXPECT_FALSE(jsval.isError());
}

TEST_F(ColorMapperJSProxyTest, GetNearestValue) {
    QJSEngine* pEngine = createScriptEngine();
    QJSValue jsval = pEngine->evaluate(
            R"JavaScript(
           var mapper = new ColorMapper({
               0xC50A08: 1,
               0x32BE44: 2,
               0x42D4F4: 3,
               0xF8D200: 4,
               0x0044FF: 5,
               0xAF00CC: 6,
               0xFCA6D7: 7,
               0xF2F2FF: 8,
           });
           /* red */
           if (mapper.getValueForNearestColor(0xFF0000) != 1) {
               throw Error();
           };
           /* blue */
           if (mapper.getValueForNearestColor(0x0000AA) != 5) {
               throw Error();
           };
           /* white */
           if (mapper.getValueForNearestColor(0xFFFFFF) != 8) {
               throw Error();
           };
           )JavaScript");
    EXPECT_FALSE(jsval.isError());
}
