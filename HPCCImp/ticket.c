#include "ticket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int compare_tickets(const void* a, const void* b) {
    const Ticket* t1 = (const Ticket*)a;
    const Ticket* t2 = (const Ticket*)b;
    const time_t time1 = mktime((struct tm*)&t1->sold_date);
    const time_t time2 = mktime((struct tm*)&t2->sold_date);
    return (int)difftime(time1, time2);
}

#define MAX_LINE_LENGTH 512

int parse_datetime_ticket(const char* str, struct tm* out_tm) {
    memset(out_tm, 0, sizeof(struct tm));

    int year, month, day, hour, min, sec;
    const int parsed = sscanf(str, "%d-%d-%d %d:%d:%d",
                        &year, &month, &day,
                        &hour, &min, &sec);
    if (parsed != 6) return -1;

    out_tm->tm_year = year - 1900;
    out_tm->tm_mon = month - 1;
    out_tm->tm_mday = day;
    out_tm->tm_hour = hour;
    out_tm->tm_min = min;
    out_tm->tm_sec = sec;
    out_tm->tm_isdst = -1;

    const time_t t = mktime(out_tm);
    if (t == -1) return -1;

    return 0;
}

int read_tickets_csv(const char* filepath, Ticket** tickets_out, int* count_out) {
    FILE* file = fopen(filepath, "r");
    if (!file) return -1;

    char line[MAX_LINE_LENGTH];
    int capacity = 100, count = 0;
    Ticket* tickets = malloc(capacity * sizeof(Ticket));
    if (!tickets) { fclose(file); return -1; }

    fgets(line, MAX_LINE_LENGTH, file);  // Skip header

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        char* parts[3];
        int i = 0;
        char* token = strtok(line, ",");
        while (token && i < 3) {
            parts[i++] = token;
            token = strtok(NULL, ",");
        }
        if (i < 3) continue;  // Not enough columns

        if (count >= capacity) {
            capacity *= 2;
            Ticket* tmp = realloc(tickets, capacity * sizeof(Ticket));
            if (!tmp) { free(tickets); fclose(file); return -1; }
            tickets = tmp;
        }

        Ticket t;
        if (parse_datetime_ticket(parts[0], &t.sold_date) != 0) continue;
        t.stop_id = atoi(parts[1]);
        t.variant_id = atoi(parts[2]);

        tickets[count++] = t;
    }

    fclose(file);

    //qsort(tickets, count, sizeof(Ticket), compare_tickets);

    *tickets_out = tickets;
    *count_out = count;
    return 0;
}

void print_ticket_line(const Ticket* ticket) {
    char date_str[20];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", &ticket->sold_date);
    printf("Sold Date: %s, Stop ID: %d, Variant ID: %d\n", date_str, ticket->stop_id, ticket->variant_id);
}

void free_tickets(Ticket* tickets) {
    if (tickets != NULL) {
        free(tickets);
    }
}
