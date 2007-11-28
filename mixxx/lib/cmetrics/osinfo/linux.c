#include <stdlib.h>
#include "osinfo.h"
#include "../globaldefs.h"
#include "../utf/utstr.h"

XCHAR *osInfoStz()
{
    XCHAR *pstz;
    pstz = malloc(100 * sizeof(XCHAR));
    xsprintf(pstz, 100, "LINUX not implemented");
    return pstz;
}
