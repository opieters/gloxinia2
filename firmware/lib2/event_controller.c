#include <stddef.h>
#include <xc.h>
#include <event_controller.h>
#include <stdbool.h>

// variables for event scheduling and handling
volatile uint16_t n_queued_tasks = 0;
task_t task_list[MAX_N_TASKS];
volatile uint16_t task_index = 0;
uint32_t id_counter = DEFAULT_ID;
volatile uint16_t n_scheduled_events = 0;
task_schedule_t schedule_list[MAX_N_SCHEDULES];


void event_controller_init(void){
    // configure scheduled event timer
    T9CONbits.TON = 0;
    T9CONbits.TCS = 0; // use internal instruction cycle as clock source
    T9CONbits.TGATE = 0; // disable gated timer
    T9CONbits.TCKPS = 0b11; // prescaler 1:256
    TMR9 = 0; // clear timer register
    PR9 = (uint16_t) ((FCY / 256) / EVENT_CONTROLLER_FREQUENCY) - 1; // set period
    _T9IF = 0; // clear interrupt flag
    _T9IE = 1; // enable interrupt

    // start timer 
    T9CONbits.TON = 1;
}

uint32_t push_queued_task(task_t task) {
    uint16_t index;
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

task_t pop_queued_task(void) {
    task_t task;

    _GIE = 0; // disable interrupts

    if (n_queued_tasks > 0) {
        n_queued_tasks--;

        task = task_list[task_index];
        task_index = (task_index + 1) % MAX_N_TASKS;
    } else {
        task.cb = task_dummy;
        task.data = NULL;
    }

    _GIE = 1; // enable interrupts

    return task;
}

void schedule_init(task_schedule_t* s,
        task_t task,
        uint16_t period){

    s->id = DEFAULT_ID;
    s->task = task;
    s->period = period;
    s->trigger_time = period-1;
    
}

uint32_t schedule_event(task_schedule_t* s) {
    uint32_t i = 0;

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


uint32_t schedule_specific_event(task_schedule_t* s, uint32_t id) {
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
        s->id = id;
        schedule_list[n_scheduled_events] = *s;
        n_scheduled_events++;
    } else {
        id = DEFAULT_ID;
    }

    _GIE = 1; // enable interrupts

    return id;
}

uint32_t schedule_remove_event(uint32_t id){
    int i, j;
    
    _GIE = 0; // disable interrupts
    
    for(i = 0; i < n_scheduled_events; i++){
        if(schedule_list[i].id == id){
            break;
        }
    }
    
    // the id was not found -> return the default id
    if(n_scheduled_events == id){
        _GIE = 1;
        
        return DEFAULT_ID;
    }
    
    // move all other events one down
    for(j = i+1; j < n_scheduled_events; j++){
        schedule_list[j-1] = schedule_list[j];
    }
    
    _GIE = 1; // enable interrupts
    
    return id;
}


void __attribute__((__interrupt__, no_auto_psv)) _T9Interrupt(void) {
    uint16_t i;
    
    for(i = 0; i < n_scheduled_events; i++){
        task_schedule_t* s = &schedule_list[i];
        
        if(s->trigger_time > 0){
            s->trigger_time--;
        } else {
            s->trigger_time = s->period;
            push_queued_task(s->task);
        }
    }
    _T9IF = 0;
}

void task_dummy(void* data){
    
}