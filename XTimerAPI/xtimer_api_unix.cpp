// xtimer_api_unix.cpp
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>

int xtimer_send(const char* command, char* outbuffer = nullptr, size_t buffersize = 0)
{
    int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    sockaddr_un addr{};
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, "/tmp/XTimerAPI");

    if (connect(fd, (const sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -2;
    }

    write(fd, command, strlen(command));

    if (outbuffer && buffersize > 0) {
        ssize_t n = read(fd, outbuffer, buffersize - 1);
        if (n < 0) {
            close(fd);
            return -3;
        }
        outbuffer[n] = '\0';
    }

    close(fd);
    return 0;
}
