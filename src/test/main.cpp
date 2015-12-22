#include <gtest/gtest.h>
#include "util/console.h"
#include "errordialoghandler.h"

int main(int argc, char **argv) {
    Console console();
    // We never want to popup error dialogs when running tests.
    ErrorDialogHandler::setEnabled(false);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
