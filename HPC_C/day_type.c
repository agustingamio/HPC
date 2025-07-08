#include "day_type.h"
#include <time.h>

DayType get_previous_day_type(DayType day){
    switch (day) {
        case WORKING_DAY:
            return SUNDAY; //TODO: Revisar si no es mejor devolver WORKING_DAY
        case SATURDAY:
            return WORKING_DAY;
        case SUNDAY:
            return SATURDAY;
        default:
            return WORKING_DAY;
    }
}

DayType get_next_day_type(DayType day){
    switch (day) {
        case WORKING_DAY:
            return SATURDAY; //TODO: Revisar si no es mejor devolver WORKING_DAY
        case SATURDAY:
            return SUNDAY;
        case SUNDAY:
            return WORKING_DAY;
        default:
            return WORKING_DAY;
    }
}

DayType get_day_type_from_date(struct tm date) {
    int day_of_week = date.tm_wday;
    
    switch (day_of_week) {
        case 0:
            return SUNDAY;
        case 6:
            return SATURDAY;
        default:
            return WORKING_DAY;
    }
}