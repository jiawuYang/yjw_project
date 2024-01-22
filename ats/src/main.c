#include "assert_check.h"
#include "ats.h"
#include "ats_chn.h"
#include "error_code.h"
#include <unistd.h>

#if 1
int main()
{
    LOG_I("ats running, compile time: %s %s", __DATE__, __TIME__);

    int ret = AtCmdInit();
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    ret = ChnStdInInit();
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    ret = ChnUdpInit();
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    while (1) {
        sleep(1);
    }

    ret = AtCmdDeInit();
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return 0;
}
#else
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int fd = open(argv[1], O_RDWR);
    CHECK_RETURN(fd > 0, RET_ERROR);

    uint8_t buf[64] = {0};
    int ret = 0;

    if (strcmp(argv[2], "r") == 0) {
        ret = read(fd, buf, sizeof(buf));
        LOG_I("read: %s", buf);
    } else if (strcmp(argv[2], "w") == 0) {
        ret = write(fd, argv[3], strlen(argv[3]));
        LOG_I("write len: %d", ret);
    }

    close(fd);

    return RET_OK;
}
#endif