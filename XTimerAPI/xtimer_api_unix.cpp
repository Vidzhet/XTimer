// xtimer_api_unix.cpp
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int xtimer_send(const char* command, char* outbuffer, size_t buffersize)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/tmp/XTimerAPI");

    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -2;
    }

    write(fd, command, strlen(command));

    if (outBuffer && outBufferSize > 0) {
        ssize_t n = read(fd, outBuffer, outBufferSize - 1);
        if (n < 0) {
            close(fd);
            return -3;
        }
        outBuffer[n] = '\0';
    }

    close(fd);
    return 0;
}
