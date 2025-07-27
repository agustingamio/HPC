#include "day_type.h"
#include <time.h>

DayType get_previous_day_type(const DayType day){
    if (day == SUNDAY)
        return SATURDAY;

    return WORKING_DAY;
}

DayType get_next_day_type(const DayType day){
    if (day == SATURDAY)
        return SUNDAY;

    return WORKING_DAY;
}

DayType get_day_type_from_date(const struct tm date) {
    const int day_of_week = date.tm_wday;
    
    switch (day_of_week) {
        case 0:
            return SUNDAY;
        case 6:
            return SATURDAY;
        default:
            return WORKING_DAY;
    }
}