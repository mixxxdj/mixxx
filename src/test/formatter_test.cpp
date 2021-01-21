#include "util/formatter.h"

#include <QDir>
#include <QtDebug>

#include "test/mixxxtest.h"

class FormatterTest : public testing::Test {
};

using namespace Grantlee;

TEST_F(FormatterTest, TestRangeGroupFilter) {
    // Generate a file name for the temporary file
    auto engine = Formatter::getEngine(nullptr);
    auto context = new Context();
    //context->insert("x1", "122");
    Template t1 = engine->newTemplate(QStringLiteral("{{143|rangegroup}}"), QStringLiteral("t1"));

    auto pattern = t1->render(context);

    EXPECT_EQ(t1->render(context),
            QString("140-150"));

    Template t2 = engine->newTemplate(QStringLiteral("{{x1|rangegroup}}"), QStringLiteral("t2"));
    context->insert("x1", "122");
    EXPECT_EQ(t2->render(context), QString("120-130"));
    context->insert(QStringLiteral("x1"), QVariant(123));
    EXPECT_EQ(t2->render(context), QString("120-130"));
    context->insert(QStringLiteral("x1"), QVariant(130));
    EXPECT_EQ(t2->render(context), QString("130-140"));
    context->insert(QStringLiteral("x1"), QVariant(QString("131")));
    EXPECT_EQ(t2->render(context), QString("130-140"));

    // FIXME(XXX) why is filter argument QVariant(Invalid) ???
#if 0
    // use different grouping size
    Template t3 = engine->newTemplate(QStringLiteral("{{ x1|rangegroup:\"5\" }}"), QStringLiteral("t3"));
    context->insert("x1", "122");
    qDebug() << t2->render(context);
    EXPECT_EQ(t2->render(context), QString("120-125"));
    context->insert("x1", QVariant(122.3));
    EXPECT_EQ(t2->render(context), QString("120-125"));
    context->insert("x1", QVariant(120.000001));
    EXPECT_EQ(t2->render(context), QString("120-125"));
    context->insert("x1", QVariant(119.99999));
    EXPECT_EQ(t2->render(context), QString("115-120"));

    // use float group size
    Template t4 = engine->newTemplate(QStringLiteral("{{x1|rangegroup:\"2.5\"}}"), QStringLiteral("t4"));

    context->insert("x1", QVariant(119.99999));
    EXPECT_EQ(t2->render(context), QString("117.5-120"));
    context->insert("x1", QVariant(120.0));
    EXPECT_EQ(t2->render(context), QString("120-122.5"));
#endif
}
