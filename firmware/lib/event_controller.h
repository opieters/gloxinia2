#ifndef __EVENT_CONTROLLER_H__
#define	__EVENT_CONTROLLER_H__

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>

#define MAX_N_TASKS 64
#define MAX_N_SCHEDULES 32
#define DEFAULT_ID 0
#define ID_READY_SCHEDULE 1

typedef void (*event_t)(void);

typedef struct task_schedule_s {
    event_t task;

    uint16_t period_s; // expressed in seconds
    unsigned int trigger_time;

    uint32_t id; // unique identifier
} task_schedule_t;

extern volatile uint16_t n_queued_tasks;
extern volatile uint16_t n_scheduled_events;
extern task_schedule_t schedule_list[MAX_N_SCHEDULES];

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    int schedule_event(task_schedule_t s);

    uint32_t schedule_specific_event(task_schedule_t s, uint32_t id);

    event_t pop_queued_task(void);

    int push_queued_task(event_t task);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

