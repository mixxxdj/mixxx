#include <gtest/gtest.h>
#include <util/borrowable_ptr.h>

#include <QtConcurrentRun>
#include <QtDebug>
#include <chrono>
#include <latch>
#include <thread>

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

    std::latch firstIterationDone(2);
    auto future1 = QtConcurrent::run([&]() {
        {
            borrowed_ptr b1 = borrowable.borrow();
            borrowed_ptr b2 = borrowable.borrow();
            int* p1 = b1.get();
            int* p2 = b2.get();
            qDebug() << "future1-first" << (p1 ? *p1 : 0) << (p2 ? *p2 : 0);
        }
        firstIterationDone.arrive_and_wait();

        for (int k = 1; k < 10; ++k) {
            borrowed_ptr b1 = borrowable.borrow();
            borrowed_ptr b2 = borrowable.borrow();
            int* p1 = b1.get();
            int* p2 = b2.get();
            qDebug() << "future1-afterSwap" << (p1 ? *p1 : 0) << (p2 ? *p2 : 0);
        }
    });

    auto future2 = QtConcurrent::run([&]() {
        {
            borrowed_ptr b1 = borrowable.borrow();
            borrowed_ptr b2 = borrowable.borrow();
            int* p1 = b1.get();
            int* p2 = b2.get();
            qDebug() << "future2-first" << (p1 ? *p1 : 0) << (p2 ? *p2 : 0);
        }
        firstIterationDone.arrive_and_wait();

        for (int k = 1; k < 10; ++k) {
            borrowed_ptr b1 = borrowable.borrow();
            borrowed_ptr b2 = borrowable.borrow();
            int* p1 = b1.get();
            int* p2 = b2.get();
            qDebug() << "future2-afterSwap" << (p1 ? *p1 : 0) << (p2 ? *p2 : 0);
        }
    });

    firstIterationDone.wait();

    borrowable = borrowable_ptr(&j);

    future1.waitForFinished();
    future2.waitForFinished();
}

} // namespace
