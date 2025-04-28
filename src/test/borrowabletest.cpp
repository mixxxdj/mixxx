#include <gtest/gtest.h>
#include <util/borrowable_ptr.h>

#include <QtConcurrentRun>
#include <QtDebug>

namespace {

class BorrowableTest : public testing::Test {
};

TEST_F(BorrowableTest, SingleThread) {
    int i = 5;
    {
        auto borrowable = borrowable_ptr(&i);
        {
            borrowed_ptr borrowed1 = borrowable.borrow();
            borrowed_ptr borrowed2 = borrowable.borrow();
        }
    }
}

TEST_F(BorrowableTest, TwoThreads) {
    int i = 1;
    int j = 2;

    auto borrowable = borrowable_ptr(&i);

    auto future1 = QtConcurrent::run([&borrowable]() {
        for (int k = 0; k < 10; ++k) {
            borrowed_ptr borrowed1 = borrowable.borrow();
            borrowed_ptr borrowed2 = borrowable.borrow();
            int* p1 = borrowed1.get();
            int* p2 = borrowed1.get();
            qDebug() << "future1" << (p1 ? *p1 : 0) << (p2 ? *p2 : 0);
        }
    });

    auto future2 = QtConcurrent::run([&borrowable]() {
        for (int k = 0; k < 10; ++k) {
            borrowed_ptr borrowed1 = borrowable.borrow();
            borrowed_ptr borrowed2 = borrowable.borrow();
            int* p1 = borrowed1.get();
            int* p2 = borrowed1.get();
            qDebug() << "future2" << (p1 ? *p1 : 0) << (p2 ? *p2 : 0);
        }
    });

    // waist a bit of time implicit sync on the stdout
    for (int i = 0; i < 5; ++i) {
        qDebug() << "main";
    }

    // replace borrowable object
    borrowable = borrowable_ptr(&j);

    // Wait for both tasks to complete
    future1.waitForFinished();
    future2.waitForFinished();
}

} // namespace
