#include <stddef.h>
#include <unistd.h>
#include <errno.h>

size_t readn(int fd, void *buffer, size_t size) {
    char *buffer_pointer = (char *)buffer;
    int length = size;

    while (length > 0) {
        int result = read(fd, buffer_pointer, length);

        if (result < 0) {
            if (errno == EINTR) // EINTR 表示此调用被信号所中断.
                continue; // 考虑非阻塞的清空，这里需要再次调用 read
            else
                return -1;
        } else if (result == 0) {
            break;  // EOF(End of File)表示套接字关闭
        }

        length -= result;
        buffer_pointer += result;
    }

    return size - length;
}