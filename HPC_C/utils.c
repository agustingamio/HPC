#include "utils.h"
#include <stdlib.h>
#include <string.h>

long time_diff_in_seconds(struct tm a, struct tm b) {
    return (a.tm_hour * 3600 + a.tm_min * 60 + a.tm_sec) -
           (b.tm_hour * 3600 + b.tm_min * 60 + b.tm_sec);
}

struct tm add_seconds(struct tm time, long seconds) {
    time_t rawtime = time.tm_hour * 3600 + time.tm_min * 60 + time.tm_sec + seconds;
    struct tm result = time;
    result.tm_hour = (rawtime / 3600) % 24;
    result.tm_min = (rawtime % 3600) / 60;
    result.tm_sec = rawtime % 60;
    return result;
}

long compare_times(struct tm a, struct tm b) {
    return time_diff_in_seconds(a, b);
}

int same_time(struct tm a, struct tm b) {
    return a.tm_hour == b.tm_hour && a.tm_min == b.tm_min && a.tm_sec == b.tm_sec;
} 