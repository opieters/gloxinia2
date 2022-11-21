#include <stddef.h>
#include <xc.h>
#include <event_controller.h>
#include <stdbool.h>

// variables for event scheduling and handling
volatile uint16_t n_queued_tasks = 0;
event_t task_list[MAX_N_TASKS];
volatile uint16_t task_index = 0;
uint32_t id_counter = DEFAULT_ID;
volatile uint16_t n_scheduled_events = 0;
task_schedule_t schedule_list[MAX_N_SCHEDULES];

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


