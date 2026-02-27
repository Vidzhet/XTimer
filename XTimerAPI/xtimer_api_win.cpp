// xtimer_api_win.cpp
#include <windows.h>

int xtimer_send(const char* command, char* outbuffer = nullptr, size_t buffersize = 0)
{
    HANDLE pipe = CreateFileA(
        "\\\\.\\pipe\\XTimerAPI",
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr,
        OPEN_EXISTING,
        0, nullptr
    );

    if (pipe == INVALID_HANDLE_VALUE)
        return -1;

    DWORD written = 0;
    if (!WriteFile(pipe, command, (DWORD)strlen(command), &written, nullptr)) {
        CloseHandle(pipe);
        return -2;
    }

    if (outbuffer && buffersize > 0) {
        DWORD readBytes = 0;
        BOOL ok = ReadFile(
            pipe,
            outbuffer,
            (DWORD)(buffersize - 1),
            &readBytes,
            nullptr
        );

        if (!ok) {
            CloseHandle(pipe);
            return -3;
        }

        outbuffer[readBytes] = '\0';
    }

    CloseHandle(pipe);
    return 0;
}
