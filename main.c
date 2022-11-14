#include <stdio.h>
#include <time.h>

int main(void) {
    char buf[1000];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    printf("Time is: [%s]\n", buf);

    return 0;
}