#include <event_controller.h>
#include <stdbool.h>
#include <stdio.h>

volatile uint16_t n_queued_tasks = 0;
event_t task_list[MAX_N_TASKS];
volatile uint16_t task_index = 0;

uint32_t id_counter = DEFAULT_ID;
volatile uint16_t n_scheduled_events = 0;
task_schedule_t schedule_list[MAX_N_SCHEDULES];

int schedule_event(task_schedule_t s) {
    int i = 0;

    _GIE = 0; // disable interrupts

    // get id
    uint32_t id = id_counter + 1;
    id_counter++;

    while (i < n_scheduled_events) {
        if (id == schedule_list[i].id) {
            i = 0;
            id++;
        } else {
            i++;
        }
    }

    id = schedule_specific_event(s, id);

    _GIE = 1; // enable interrupts

    return id;
}

uint32_t schedule_specific_event(task_schedule_t s, uint32_t id) {
    bool unique_id = true;
    int i;

    // the default ID cannot be scheduled
    if (id == DEFAULT_ID) {
        return DEFAULT_ID;
    }

    _GIE = 0; // disable interrupts

    // check if id is unique
    for (i = 0; i < n_scheduled_events; i++) {
        if (schedule_list[i].id == id) {
            unique_id = false;
            break;
        }
    }

    // if the id is unique, schedule. Otherwise indicate error by means of the
    // default ID
    if (unique_id) {
        s.id = id;
        schedule_list[n_scheduled_events] = s;
        n_scheduled_events++;
    } else {
        id = DEFAULT_ID;
    }

    _GIE = 1; // enable interrupts

    return id;
}

event_t pop_queued_task(void) {
    event_t task;

    _GIE = 0; // disable interrupts

    if (n_queued_tasks > 0) {
        n_queued_tasks--;

        task = task_list[task_index];
        task_index = (task_index + 1) % MAX_N_TASKS;
    } else {
        task = NULL;
    }

    _GIE = 1; // enable interrupts

    return task;
}

int push_queued_task(event_t task) {
    int index;
    _GIE = 0; // disable interrupts

    if (n_queued_tasks < MAX_N_TASKS) {

        index = task_index + n_queued_tasks;

        n_queued_tasks++;

        task_list[index] = task;
    } else {
        _GIE = 1;

        return 1;
    }

    _GIE = 1; // enable interrupts

    return 0;
}