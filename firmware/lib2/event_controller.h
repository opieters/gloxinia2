#ifndef __EVENT_CONTROLLER_H__
#define __EVENT_CONTROLLER_H__

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

/// @brief Maximum number of tasks that can be queued.
#define MAX_N_TASKS 64

/// @brief Maximum number of scheduled events that can be queued.
#define MAX_N_SCHEDULES 32

/// @brief Default ID for a scheduled event.
#define DEFAULT_ID 0

/// @brief Reserved ID for the schedule that sends the ready message.
#define ID_READY_SCHEDULE 1

/// @brief Reserved ID for the schedule that gets the address.
#define ID_GET_ADDRESS 2

/// @brief Reserved ID for the schedule that stores the node config data to the
/// SD card and starts the sensor readout in each node.
#define ID_DICIO_BORADCAST_START 3

/// @brief Reserved ID for the schedule that stops the sensor readout in each 
/// node.
#define ID_DICIO_BORADCAST_STOP 4

#define ID_UART_OVERFLOW_SCHEDULE       5

#define ID_SEARCH_START (0x10)

/// @brief The frequency of the event controller in Hz.
#define EVENT_CONTROLLER_FREQUENCY 10

typedef enum {
    TASK_STATUS_DONE,
    TASK_STATUS_INITIALISED,
    TASK_STATUS_QUEUED,
    TASK_STATUS_EXEC,
} task_status_t;

/**
 * @brief A task is a function that takes no arguments and returns void and is executed at a certain time.
 *
 * @param cb: The function to be executed.
 * @param data: The data to be passed to the function.
 */
typedef struct
{
    void (*cb)(void *data);
    void (*data)(void);
    task_status_t status;
} task_t;

/**
 * @brief A scheduled event is a task that is executed at a specific time and with a specific frequency.
 *
 * @details Schedules are limited to MAX_N_SCHEDULES and are stored in a buffer. Each schedule has a unique ID.
 *
 * @param task: The task to be executed.
 * @param period: The period of the task in multiples of the base period (0.1 second).
 * @param trigger_time: The time at which the task should be executed. Updated automatically.
 * @param id: The unique identifier of the schedule.
 */
typedef struct task_schedule_s
{
    task_t task;

    uint16_t period;
    uint16_t trigger_time;

    uint32_t id;
    
    task_t* queued_task;
} task_schedule_t;

/// @brief The number of tasks that are currently queued.
extern volatile uint16_t n_queued_tasks;

/// @brief The buffer of schedules that are currently in use.
extern volatile uint16_t n_scheduled_events;

/// @brief The schedule buffer.
extern task_schedule_t schedule_list[MAX_N_SCHEDULES];

#ifdef __cplusplus
extern "C"
{
#endif
    
    void task_init(task_t* task, void (*cb)(void *data), void (*data)(void));
    void task_cleanup(task_t* task);

    /**
     * @brief Initializes the event controller.
     *
     * @details Initializes the event controller and the schedule buffer. Also
     * starts the timer associated with the event controller. Scheduled events
     * are automatically added to the event queue, but are not handled
     * automatically! In the main loop, n_queued_tasks should be checked and
     * tasks in the queue popped for execution.
     */
    void event_controller_init(void);

    /**
     * @brief Schedules a task to be executed at a specific time.
     *
     * @param s: The schedule to be initialised.
     * @param task: The task to be executed.
     * @param period: The period of the task in multiples of the base period (0.1 second).
     */
    void schedule_init(task_schedule_t *s, task_t task, uint16_t period);

    /**
     * @brief Schedules an event to be executed at a specific time.
     *
     * @details During scheduling, interrupts are disabled.
     *
     * @param s: The schedule to be initialised.
     * @return The ID of the schedule. DEFAULT_ID is returned if was not
     * possible to add the schedule.
     */
    uint32_t schedule_event(task_schedule_t *s);

    /**
     * @brief Schedules an event to be executed with a specific ID.
     *
     * @param s: The schedule to be initialised.
     * @param id: The ID of the schedule, should not be DEFAULT_ID
     * @return The ID of the schedule. DEFAULT_ID is returned if the ID is
     * invalid or it is already in use.
     */
    uint32_t schedule_specific_event(task_schedule_t *s, uint32_t id);

    /**
     * @brief Removes a scheduled event.
     *
     * @param id: The ID of the schedule to be removed.
     * @return The ID of the schedule. DEFAULT_ID is returned if the schedule
     * with ID id was not found.
     */
    uint32_t schedule_remove_event(uint32_t id);

    /**
     * @brief Fetches the next task in the queue.
     *
     * @details decreases n_queued_tasks by 1.
     * @return The next task to execute.
     */
    task_t* pop_queued_task(void);

    /**
     * @brief Adds a task to the queue.
     *
     * @details increases n_queued_tasks by 1.
     *
     * @param task: The task to be added to the queue.
     * @return true if the task was added to the queue, false otherwise.
     */
    task_t* push_queued_task(const task_t* task);

    /**
     * @brief A dummy task that does nothing.
     */
    void task_dummy(void *data);

#ifdef __cplusplus
}
#endif

#endif
