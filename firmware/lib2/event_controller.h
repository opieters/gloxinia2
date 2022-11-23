#ifndef __EVENT_CONTROLLER_H__
#define	__EVENT_CONTROLLER_H__

#include <xc.h>  
#include <stdint.h>

#define MAX_N_TASKS 64
#define MAX_N_SCHEDULES 32
#define DEFAULT_ID 0
#define ID_READY_SCHEDULE 1
#define ID_GET_ADDRESS 2

#define EVENT_CONTROLLER_FREQUENCY 10

typedef struct {
    void (*cb)(void* data);
    void (*data)(void);
} task_t; 

typedef struct task_schedule_s {
    task_t task;

    uint16_t period; // expressed in multiples of the base period (0.1 second)
    uint16_t trigger_time;

    uint32_t id; // unique identifier
} task_schedule_t;

extern volatile uint16_t n_queued_tasks;
extern volatile uint16_t n_scheduled_events;
extern task_schedule_t schedule_list[MAX_N_SCHEDULES];

#ifdef	__cplusplus
extern "C" {
#endif
    
    void event_controller_init(void);
    
    void schedule_init(task_schedule_t* s, task_t task, uint16_t period);

    uint32_t schedule_event(task_schedule_t* s);

    uint32_t schedule_specific_event(task_schedule_t* s, uint32_t id);
    
    uint32_t schedule_remove_event(uint32_t id);

    task_t pop_queued_task(void);

    uint32_t push_queued_task(task_t task);
    
    void task_dummy(void* data);

#ifdef	__cplusplus
}
#endif

#endif

