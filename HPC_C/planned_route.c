#include "planned_route.h"
#include "day_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int error = set_times_from_last_stop(&routes, count);
    if (error == 0){
        *lines_out = routes;
        *count_out = count;
        return 0;
    }else{
        return -1;
    }
}

int set_times_from_last_stop( PlannedRouteCsvLine** routes, int* routes_count){
    //Recorrer cada tramo
    for(int i = 0; i<routes_count; i++){
        if (routes[i]->relative_stop_id == 1){
            routes[i]->time_from_last_stop = 0;
        }else{
            PlannedRouteCsvLine* previous_route = get_previous_route_node(routes, routes_count, routes[i]);
            if(previous_route!= NULL)
            routes[i]->time_from_last_stop = time_diff_in_seconds(routes[i]->hour, previous_route->hour);
        }
    }
}

PlannedRouteCsvLine* get_previous_route_node(PlannedRouteCsvLine** routes, int* routes_count, PlannedRouteCsvLine *route){
    if(route->last_day == 'N'){
        PlannedRouteCsvLine* previous_route;
        for(int i = 0; i<routes_count; i++){
            PlannedRouteCsvLine* current_route = routes[i]; 
            if((current_route->variant_id = route->variant_id) && (same_time(current_route->frequency,route->frequency)) && (current_route->relative_stop_id == route->relative_stop_id-1) && (current_route->day_type == route->day_type))
                previous_route = current_route;
                // TODO : Mem copy para que no referencie al mismo
                return previous_route;
        }
    }else{
        for(int i = 0; i<routes_count; i++){
            PlannedRouteCsvLine* current_route = routes[i]; 
            PlannedRouteCsvLine** previous_routes;
            int count_previous = 0;

            if((current_route->variant_id = route->variant_id) && (same_time(current_route->frequency,route->frequency)) && (current_route->relative_stop_id == route->relative_stop_id-1)){
                previous_routes[count_previous] = current_route;
                count_previous ++;
            }

            for(int j = 0; j< count_previous; j++){
                if((current_route->day_type == previous_routes[j]->day_type) && previous_routes[j]->last_day=='S'){
                    //TODO: Mem copy para que no referencie al mismo
                    return previous_routes[j]; 
                }else{
                    if((current_route->day_type == previous_routes[j]->day_type) && previous_routes[j]->last_day=='N')
                        for(int k = 0; k< count_previous; k++){
                            if(previous_routes[k]->day_type = get_previous_day_type(current_route->day_type)){
                                //TODO : Mem copy
                                return previous_routes[k];
                            }
                        }
                }
            }
        }
    }
    
    
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