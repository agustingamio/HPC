#ifndef DAY_TYPE_H
#define DAY_TYPE_H
#include "time.h"

typedef enum {
    WORKING_DAY = 1,
    SATURDAY = 2,
    SUNDAY = 3
} DayType;

// Devuelve el tipo de día anterior en la secuencia circular.
DayType get_previous_day_type(DayType day);

// Devuelve el tipo de día siguiente en la secuencia circular.
DayType get_next_day_type(DayType day);

// Determina el tipo de día (WORKING_DAY, SATURDAY, SUNDAY) a partir de un `struct tm`.
DayType get_day_type_from_date(struct tm date);

#endif