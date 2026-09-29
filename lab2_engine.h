#ifndef LAB2_ENGINE_H
#define LAB2_ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STUDENT_ID_SEED 400442107u
#define RUNLENGTH 200000L
#define WARMUP 10000L
#define MAX_EVENTS 1000000

#define VOICE_PERIOD 0.020
#define VOICE_SERVICE (1776.0 / 1000000.0)

typedef struct Packet {
    double arrival_time;
    double service_time;
    int source;
    int voice;
    struct Packet *next;
} Packet;

typedef struct {
    Packet *front;
    Packet *back;
} Queue;

typedef struct {
    double time;
    int type;
    int station;
    Packet *packet;
} Event;

static Event events[MAX_EVENTS];
static int event_count;
static double clock_time;

static Queue waiting[3][2];
static Packet *in_service[3][2];

static int current_part;
static double current_rate;
static double current_p12;

static long completed;
static long measured[3];
static long over_20ms;
static double total_delay[3];

static double uniform_random(void)
{
    return ((double)rand() + 0.5) / ((double)RAND_MAX + 1.0);
}

static double exponential_random(double mean)
{
    return -mean * log(uniform_random());
}

static void schedule(double time, int type, int station, Packet *packet)
{
    Event e;
    int i;

    if (event_count + 1 >= MAX_EVENTS) {
        fprintf(stderr, "Event list full\n");
        exit(1);
    }

    e.time = time;
    e.type = type;
    e.station = station;
    e.packet = packet;

    i = ++event_count;

    while (i > 1 && events[i / 2].time > time) {
        events[i] = events[i / 2];
        i /= 2;
    }

    events[i] = e;
}

static Event next_event(void)
{
    Event result;
    Event last;
    int i;
    int child;

    result = events[1];
    last = events[event_count--];
    i = 1;

    while (2 * i <= event_count) {
        child = 2 * i;

        if (child < event_count &&
            events[child + 1].time < events[child].time) {
            child++;
        }

        if (events[child].time >= last.time) {
            break;
        }

        events[i] = events[child];
        i = child;
    }

    if (event_count > 0) {
        events[i] = last;
    }

    return result;
}

static void enqueue(Queue *q, Packet *p)
{
    p->next = NULL;

    if (q->back) {
        q->back->next = p;
    } else {
        q->front = p;
    }

    q->back = p;
}

static Packet *dequeue(Queue *q)
{
    Packet *p;

    p = q->front;

    if (p) {
        q->front = p->next;

        if (!q->front) {
            q->back = NULL;
        }

        p->next = NULL;
    }

    return p;
}

static Packet *new_packet(int source, int voice, double service)
{
    Packet *p;

    p = (Packet *)malloc(sizeof(Packet));

    if (!p) {
        perror("malloc");
        exit(1);
    }

    p->arrival_time = clock_time;
    p->service_time = service;
    p->source = source;
    p->voice = voice;
    p->next = NULL;

    return p;
}

static int number_of_links(int station)
{
    if (current_part == 3 && station == 0) {
        return 2;
    }

    return 1;
}

static void start_waiting_packets(int station)
{
    int link;
    Packet *p;

    for (link = 0; link < number_of_links(station); link++) {
        if (in_service[station][link]) {
            continue;
        }

        /* In Part 6, queue 0 contains priority voice packets. */
        p = dequeue(&waiting[station][0]);

        if (!p) {
            p = dequeue(&waiting[station][1]);
        }

        if (!p) {
            break;
        }

        in_service[station][link] = p;

        schedule(
            clock_time + p->service_time,
            2,
            station,
            p
        );
    }
}

static void enter_station(int station, Packet *p)
{
    int queue_number;

    if (current_part == 6 && p->voice) {
        queue_number = 0;
    } else {
        queue_number = 1;
    }

    enqueue(&waiting[station][queue_number], p);
    start_waiting_packets(station);
}

static void finish_packet(Packet *p)
{
    int source;
    double delay;

    completed++;

    if (completed > WARMUP) {
        source = p->source;
        delay = clock_time - p->arrival_time;

        measured[source]++;
        total_delay[source] += delay;

        if (current_part == 2 && delay > 0.020) {
            over_20ms++;
        }
    }

    free(p);
}

static void run_simulation(
    int part,
    double rate,
    double p12,
    unsigned seed
)
{
    int s;
    int q;
    int link;
    int destination;
    int source;

    double arrival_rate;
    double service;
    double mean_ms;
    double probability;

    Packet *p;
    Event e;

    current_part = part;
    current_rate = rate;
    current_p12 = p12;

    srand(seed ? seed : 1u);

    event_count = 0;
    clock_time = 0;
    completed = 0;
    over_20ms = 0;

    for (s = 0; s < 3; s++) {
        measured[s] = 0;
        total_delay[s] = 0;

        for (q = 0; q < 2; q++) {
            waiting[s][q].front = NULL;
            waiting[s][q].back = NULL;
            in_service[s][q] = NULL;
        }
    }

    if (part == 4) {
        for (s = 0; s < 3; s++) {
            schedule(0, 0, s, NULL);
        }
    } else {
        schedule(0, 0, 0, NULL);
    }

    if (part == 5 || part == 6) {
        schedule(0, 1, 0, NULL);
    }

    while (completed < RUNLENGTH + WARMUP) {
        e = next_event();
        clock_time = e.time;

        if (e.type == 0) {
            /* Packet arrival */

            if (part == 4) {
                arrival_rate = (e.station == 0) ? 750.0 : 500.0;
            } else {
                arrival_rate = current_rate;
            }

            schedule(
                clock_time + exponential_random(1.0 / arrival_rate),
                0,
                e.station,
                NULL
            );

            if (part == 2) {
                service = 2000.0 / 1000000.0;
            } else if (part == 3) {
                service = 500.0 / 500000.0;
            } else if (part == 4) {
                if (e.station == 0) {
                    service = 1000.0 / 2000000.0;
                } else {
                    service = 1000.0 / 1000000.0;
                }
            } else if (part == 5 || part == 6) {
                service = exponential_random(0.040);
            } else {
                service = 500.0 / 1000000.0;
            }

            source = (part == 4) ? e.station : 0;

            enter_station(
                e.station,
                new_packet(source, 0, service)
            );

        } else if (e.type == 1) {
            /* Voice arrival */

            schedule(
                clock_time + VOICE_PERIOD,
                1,
                0,
                NULL
            );

            enter_station(
                0,
                new_packet(1, 1, VOICE_SERVICE)
            );

        } else {
            /* Transmission completed */

            link = 0;

            while (
                link < number_of_links(e.station) &&
                in_service[e.station][link] != e.packet
            ) {
                link++;
            }

            if (link == number_of_links(e.station)) {
                fprintf(
                    stderr,
                    "Departure did not match a link\n"
                );
                exit(1);
            }

            in_service[e.station][link] = NULL;

            if (part == 4 && e.station == 0) {
                /* Link 1 finished; route to Switch 2 or 3. */

                if (uniform_random() < current_p12) {
                    destination = 1;
                } else {
                    destination = 2;
                }

                e.packet->service_time =
                    1000.0 / 1000000.0;

                enter_station(destination, e.packet);
            } else {
                finish_packet(e.packet);
            }

            start_waiting_packets(e.station);
        }
    }

    printf(
        "%d,%.6f,%.4f,%u",
        part,
        rate,
        p12,
        seed
    );

    for (s = 0; s < 3; s++) {
        if (measured[s] > 0) {
            mean_ms =
                1000.0 * total_delay[s] / measured[s];

            printf(
                ",%.6f,%ld",
                mean_ms,
                measured[s]
            );
        } else {
            printf(",nan,0");
        }
    }

    if (part == 2) {
        probability =
            (double)over_20ms / measured[0];

        printf(",%.8f\n", probability);
    } else {
        printf(",nan\n");
    }

    for (s = 0; s < 3; s++) {
        for (q = 0; q < 2; q++) {
            while (
                (p = dequeue(&waiting[s][q])) != NULL
            ) {
                free(p);
            }

            if (in_service[s][q]) {
                free(in_service[s][q]);
                in_service[s][q] = NULL;
            }
        }
    }
}

static void print_header(void)
{
    puts(
        "part,arrival_rate_per_s,p12,seed,"
        "source1_mean_ms,source1_count,"
        "source2_mean_ms,source2_count,"
        "source3_mean_ms,source3_count,"
        "p_delay_gt_20ms"
    );
}

#endif