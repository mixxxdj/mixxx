#include "log.h"

#define CATMODULE "test"

#define LOG_ERR(x, y, z...) log_write(x, 1, CATMODULE "/" __FUNCTION__, y, ##z)


int main(void)
{
    int lid;

    log_initialize();

    lid = log_open("test.log");

    LOG_ERR(lid, "The log id is %d, damnit...", lid);

    log_close(lid);

    log_shutdown();
}
