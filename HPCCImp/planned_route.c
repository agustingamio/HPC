#include "planned_route.h"
#include "day_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define MAX_LINE_LENGTH 256

static void parse_time(const char* src, struct tm* t) {
    if (strlen(src) > 5) {
        fprintf(stderr, "Error: Invalid time format '%s'\n", src);
        return;
    }

    if (strlen(src) < 4) {
        char padded[6] = "0000";
        strncpy(padded + 4 - strlen(src), src, strlen(src));
        padded[5] = '\0';
        src = padded;
    }

    memset(t, 0, sizeof(struct tm));

    int hour = 0, min = 0;
    sscanf(src, "%2d%2d", &hour, &min);
    t->tm_hour = hour;
    t->tm_min = min;
}

static void parse_frequency(char* src, struct tm* t) {
    const int length = strlen(src);
    src[length - 1] = '\0';

    return parse_time(src, t);
}

int read_planned_route_csv(const char* filepath, PlannedRouteCsvLine** lines_out, int* count_out) {
    FILE* file = fopen(filepath, "r");
    if (!file) return -1;

    char line[MAX_LINE_LENGTH];
    int capacity = 100, count = 0;
    PlannedRouteCsvLine* routes = malloc(capacity * sizeof(PlannedRouteCsvLine));
    if (!routes) { fclose(file); return -1; }

    fgets(line, MAX_LINE_LENGTH, file); // skip header

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        char* parts[7];
        int i = 0;
        char* token = strtok(line, ";");
        while (token && i < 7) {
            parts[i++] = token;
            token = strtok(NULL, ";");
        }
        if (i < 7) continue;

        if (count >= capacity) {
            capacity *= 2;
            PlannedRouteCsvLine* tmp = realloc(routes, capacity * sizeof(PlannedRouteCsvLine));
            if (!tmp) { free(routes); fclose(file); return -1; }
            routes = tmp;
        }

        PlannedRouteCsvLine pr;
        pr.day_type = (DayType)atoi(parts[0]);
        pr.variant_id = atoi(parts[1]);
        parse_frequency(parts[2], &pr.frequency);
        pr.stop_id = atoi(parts[3]);
        pr.relative_stop_id = atoi(parts[4]);
        parse_time(parts[5], &pr.hour);
        pr.last_day = parts[6][0];
        if (pr.relative_stop_id == 1) {
            pr.departure_time = pr.hour;
            pr.has_departure_time = 1;
        } else {
            memset(&pr.departure_time, 0, sizeof(struct tm));
            pr.has_departure_time = 0;
        }
        pr.time_from_last_stop = -1;
        routes[count++] = pr;
    }

    fclose(file);
    *lines_out = routes;
    *count_out = count;
    return 0;
}

int compare_by_relative_stop_id(const void *a, const void *b) {
    const PlannedRouteCsvLine *routeA = *(const PlannedRouteCsvLine **)a;
    const PlannedRouteCsvLine *routeB = *(const PlannedRouteCsvLine **)b;

    return routeA->relative_stop_id - routeB->relative_stop_id;
}

typedef struct {
    int variant_id;
    int frequency_val;
    DayType day_type;
    char last_day;
} GroupKey;

typedef struct {
    GroupKey key;
    PlannedRouteCsvLine** routes;
    int count;
    int capacity;
} RouteGroup;

RouteGroup* find_group(RouteGroup* groups, const int group_count, const GroupKey* key) {
    for (int i = 0; i < group_count; i++) {
        if (groups[i].key.variant_id == key->variant_id &&
            groups[i].key.frequency_val == key->frequency_val &&
            groups[i].key.day_type == key->day_type &&
            groups[i].key.last_day == key->last_day) {
            return &groups[i];
            }
    }
    return NULL;
}

int get_planned_routes_from_csv(const char* filepath, PlannedRouteCsvLine** output, int* count_out) {
    PlannedRouteCsvLine* all_routes = NULL;
    int total_routes = 0;
    if (read_planned_route_csv(filepath, &all_routes, &total_routes) != 0) return -1;

    RouteGroup* groups = NULL;
    int group_count = 0;
    int group_capacity = 100;
    groups = malloc(group_capacity * sizeof(RouteGroup));

    if (!groups) return -1;

    for (int i = 0; i < total_routes; ++i) {
        PlannedRouteCsvLine* route = &all_routes[i];

        const GroupKey key = {
            .variant_id = route->variant_id,
            .frequency_val = get_secs_from_time(&route->frequency),
            .day_type = route->day_type,
            .last_day = route->last_day
        };

        // Find or create group
        int found = 0;
        int idx;
        for (int j = 0; j < group_count; ++j) {
            if (groups[j].key.variant_id == key.variant_id &&
                groups[j].key.frequency_val == key.frequency_val &&
                groups[j].key.day_type == key.day_type &&
                groups[j].key.last_day == key.last_day) {
                found = 1;
                idx = j;
                break;
            }
        }

        if (!found) {
            if (group_count >= group_capacity) {
                group_capacity *= 2;
                RouteGroup* tmp = realloc(groups, group_capacity * sizeof(RouteGroup));
                if (!tmp) {
                    return -1;
                }
                groups = tmp;
            }

            groups[group_count].key = key;
            groups[group_count].routes = malloc(100 * sizeof(PlannedRouteCsvLine*));
            groups[group_count].count = 0;
            groups[group_count].capacity = 100;
            idx = group_count++;
        }

        // Add to group
        RouteGroup* group = &groups[idx];
        if (group->count >= group->capacity) {
            group->capacity *= 2;
            group->routes = realloc(group->routes, group->capacity * sizeof(PlannedRouteCsvLine*));
        }
        group->routes[group->count++] = route;
    }

    // Sort each group by relative_stop_id
    for (int i = 0; i < group_count; ++i) {
        RouteGroup* group = &groups[i];
        qsort(group->routes, group->count, sizeof(PlannedRouteCsvLine*), compare_by_relative_stop_id);
    }

    // Compute time_from_last_stop
    for (int i = 0; i < group_count; ++i) {
        RouteGroup* group = &groups[i];
        int day_type_changed = 0;

        for (int j = 0; j < group->count; ++j) {
            PlannedRouteCsvLine* actual_stop = group->routes[j];

            if (actual_stop->relative_stop_id == 1) {
                actual_stop->time_from_last_stop = 0;
                continue;
            }

            PlannedRouteCsvLine* previous_stop = NULL;

            if (actual_stop->last_day == 'S' && !day_type_changed) {
                DayType prev_day = get_previous_day_type(actual_stop->day_type);
                GroupKey prev_key = { actual_stop->variant_id, get_secs_from_time(&actual_stop->frequency), prev_day, 'N' };

                RouteGroup* prev_group = find_group(groups, group_count, &prev_key);
                if (prev_group) {
                    previous_stop = prev_group->routes[actual_stop->relative_stop_id - 2];
                    day_type_changed = 1;
                }

            } else if (actual_stop->last_day == 'S') {
                GroupKey key = { actual_stop->variant_id, get_secs_from_time(&actual_stop->frequency), actual_stop->day_type, 'S' };
                RouteGroup* prev_group = find_group(groups, group_count, &key);
                if (prev_group) {
                    int offset = actual_stop->relative_stop_id - 1 - prev_group->routes[0]->relative_stop_id;
                    previous_stop = prev_group->routes[offset];
                    day_type_changed = 1;
                }

            } else if (actual_stop->last_day == '*') {
                GroupKey key = { actual_stop->variant_id, get_secs_from_time(&actual_stop->frequency), actual_stop->day_type, '*' };
                RouteGroup* prev_group = find_group(groups, group_count, &key);

                if (prev_group && prev_group->routes[0]->relative_stop_id == actual_stop->relative_stop_id) {
                    GroupKey fallback = { actual_stop->variant_id, get_secs_from_time(&actual_stop->frequency), actual_stop->day_type, 'N' };
                    prev_group = find_group(groups, group_count, &fallback);
                }

                if (prev_group) {
                    int offset = actual_stop->relative_stop_id - 1 - prev_group->routes[0]->relative_stop_id;
                    previous_stop = prev_group->routes[offset];
                }

            } else {
                GroupKey key = { actual_stop->variant_id, get_secs_from_time(&actual_stop->frequency), actual_stop->day_type, 'N' };
                RouteGroup* prev_group = find_group(groups, group_count, &key);
                if (prev_group) {
                    previous_stop = prev_group->routes[actual_stop->relative_stop_id - 2];
                }
            }

            if (previous_stop) {
                long secs = time_diff_in_seconds(actual_stop->hour, previous_stop->hour);
                if (secs < 0) {
                    secs += 24 * 60 * 60; // Adjust for negative time difference
                }

                actual_stop->time_from_last_stop = secs;
            }
        }
    }

    *output = all_routes;
    *count_out = total_routes;
    return 0;
}

int get_departures(const char* filepath, PlannedRouteCsvLine** departures, int* departures_count) {
    int count_out = 0;
    PlannedRouteCsvLine* lines_out = NULL;
    const int err = read_planned_route_csv(filepath, &lines_out, &count_out);

    if (err != 0)
        return err;

    *departures_count = 0;
    for (int i = 0; i < count_out; i++) {
        if (lines_out[i].relative_stop_id == 1) {
            (*departures_count)++;
        }
    }

    *departures = malloc(*departures_count * sizeof(PlannedRouteCsvLine));
    *departures_count = 0;
    for (int i = 0; i < count_out; i++) {
        if (lines_out[i].relative_stop_id == 1) {
            (*departures)[*departures_count] = lines_out[i];
            (*departures_count)++;
        }
    }

    return 0;
}

void print_planned_route_line(const PlannedRouteCsvLine* line) {
    char freq_str[6], hr_str[6], dep_str[6];
    strftime(freq_str, sizeof(freq_str), "%H:%M", &line->frequency);
    strftime(hr_str, sizeof(hr_str), "%H:%M", &line->hour);
    strftime(dep_str, sizeof(dep_str), "%H:%M", &line->departure_time);
    printf("DayType=%d, VariantId=%d, Frequency=%s, StopId=%d, RelativeStopId=%d, Hour=%s, LastDay=%c, DepartureTime=%s, HasDepartureTime=%d, TimeFromLastStop=%ld\n",
           line->day_type, line->variant_id, freq_str, line->stop_id,
           line->relative_stop_id, hr_str, line->last_day,
           dep_str, line->has_departure_time, line->time_from_last_stop);
}

void free_planned_route(PlannedRouteCsvLine* lines) {
    if (lines != NULL) {
        free(lines);
    }
}