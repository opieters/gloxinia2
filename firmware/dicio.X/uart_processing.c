#include "can.h"
#include "uart_processing.h"
#include <stdbool.h>
#include <address.h>
#include "actuators.h"
#include "sensors.h"


extern uart_message_t uart_rx_queue[UART_RX_BUFFER_SIZE];
extern volatile size_t uart_rx_read_idx;
extern uart_message_t uart_loopback;
extern uint8_t uart_loopback_data[PRINT_BUFFER_LENGTH];

volatile uint8_t start_sensors_init = 0;

extern bool can_bus_active;


void uart_rx_command_cb_dicio(){
    can_message_t can_m;
    uint8_t can_data[CAN_MAX_N_BYTES], sensor_index;
    uart_message_t m = uart_rx_queue[uart_rx_read_idx];
    bool status;
    
    uart_rx_read_idx = (uart_rx_read_idx + 1) % UART_RX_BUFFER_SIZE;
    
    if(m.status != UART_MSG_RECEIVED){
        n_uart_rx_messages--;
        return;
        // TODO: handle errors over here
    }
    
    // prepare CAN message if needed
    can_init_message(&can_m, controller_address, 
            CAN_NO_REMOTE_FRAME, CAN_EXTENDED_FRAME, m.extended_id, can_data, 0);
    
    switch(m.command){
        case SERIAL_START_MEAS_CMD:
            can_m.data_length = 1;
            can_data[0] = CAN_INFO_MSG_START_MEAS;
            can_send_message_any_ch(&can_m);
            delay_ms(1);
#ifdef __LOG__
            uart_simple_print("Sampling...");
#endif
            //start_sampling();
            break;
        case SERIAL_STOP_MEAS_CMD:
            // measurements stop immediately, reset device 
            asm ("RESET");
            break;
        case SERIAL_SENSOR_ACTIVATE_CMD:
            can_m.data_length = 2;
            can_data[0] = CAN_MSG_SENSOR_STATUS;
            can_data[1] = 1;
            can_send_message_any_ch(&can_m);
            break;
        case SERIAL_SENSOR_DEACTIVATE_CMD:
            can_m.data_length = 2;
            can_data[0] = CAN_MSG_SENSOR_STATUS;
            can_data[1] = 0;
            can_send_message_any_ch(&can_m);
            break;
        case SERIAL_RESET_NODE_CMD:
            if(m.length == 1){
                if(m.data[0] == controller_address){
                    i2c_empty_queue();
                    asm ("RESET");
                } else {
                    can_m.data_length = 2;
                    can_data[0] = CAN_INFO_MSG_RESET;
                    can_data[1] = m.data[0];
                    can_send_message_any_ch(&can_m);
                }
            }
            break;
        case SERIAL_RESET_SYSTEM_CMD:   
#ifdef __LOG__
            sprintf(print_buffer, "Resetting system...");
            uart_print(print_buffer, strlen(print_buffer));
#endif
            // send reset message to other nodes
            if(can_bus_active){
                can_m.data_length = 1;
                can_data[0] = CAN_INFO_MSG_RESET;
                while(can_send_message_any_ch(&can_m) != CAN_NO_ERROR);

                while(C1TR01CONbits.TXREQ0 == 1);
                while(C1TR01CONbits.TXREQ1 == 1);
                while(C1TR23CONbits.TXREQ2 == 1);
                while(C1TR23CONbits.TXREQ3 == 1);
                while(C1TR45CONbits.TXREQ4 == 1);
                while(C1TR45CONbits.TXREQ5 == 1);
                while(C1TR67CONbits.TXREQ6 == 1);
                while(C1TR67CONbits.TXREQ7 == 1);
                
                delay_us(100);
            }
            
            asm ("RESET");
            
            break;
        case SERIAL_TEXT_MESSAGE_CMD:
            // this message can only be sent to the computer
            break;
        case SERIAL_SENSOR_DATA_CMD:
            // this message can only be sent to the computer
            break;
        case SERIAL_SENSOR_STATUS_CMD:
            // this message can only be sent to the computer
            break;
        case SERIAL_MEAS_PERIOD_CMD:
            if(m.length == 5){
                uint8_t prescaler, i;
                uint32_t timer_period = 0;
                    
                for(i = 0; i < 4; i++){
                    timer_period = (timer_period << 8) | m.data[i];
                }
                prescaler = m.data[4] & 0b11;
                
                // TODO: add support for measurement period alterations
                //set_sample_period(timer_period, prescaler);
            }
            break;
        case SERIAL_ERR_MESSAGE_CMD:
            // this message can only be sent to the computer
            break;
        case SERIAL_LOOP_MESSAGE_CMD: {
            size_t i;
            
            uart_await_tx(&uart_loopback);
            
            for(i = 0; i < m.length; i++){
                uart_loopback_data[i] = m.data[i];
            }
            uart_init_message(&uart_loopback, 
                SERIAL_LOOP_MESSAGE_CMD,
                controller_address,
                0,            
                uart_loopback_data,
                m.length);
            
            uart_queue_message(&uart_loopback);
            
            }
            break;
        case SERIAL_ACTUATOR_STATUS:
            break;
        case SERIAL_HELLO_CMD:
#ifdef __LOG__
            sprintf(print_buffer, "Broadcasting hello message...");
            uart_print(print_buffer, strlen(print_buffer));
#endif
            
            // prepare data contents
            can_m.data_length = 1;
            can_data[0] = SERIAL_HELLO_CMD;
            
            // broadcast over bus
            can_send_message_any_ch(&can_m);
            break;
        case SERIAL_INIT_SAMPLING_CMD:
#ifdef __LOG__
            sprintf(print_buffer, "Initialising sampling event.");
            uart_print(print_buffer, strlen(print_buffer));
#endif
            //init_sampling();
            break;
        case SERIAL_INIT_SENSORS_CMD:
            can_m.data_length = 1;
            can_data[0] = CAN_INFO_MSG_INIT_SENSORS;
            
            can_send_message_any_ch(&can_m);
            break;
        case SERIAL_LIA_GAIN_SET_CMD:
            can_m.data_length = 4;
            can_data[0] = CAN_INFO_LIA_GAIN_SET;
            can_data[1] = m.data[0];
            can_data[2] = m.data[1];
            can_data[3] = m.data[2];
            
            can_send_message_any_ch(&can_m);
            break;
        case SERIAL_START_INIT:
            start_sensors_init = 1;
            break;
        case SERIAL_SENSOR_ACTUATOR_ENABLE:
            if(m.length < 2){
                uart_simple_print("UART: sensor/actuator incomplete message.");
                break;
            }
            sensor_index = m.data[1];
           /* switch(m.data[0]){
                case CAN_CONFIG_CMD_APDS9301:
                    if(sensor_index>=N_SENSOR_APDS9301){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        apds9301_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_OPT3001Q1:
                    if(sensor_index>=N_SENSOR_OPT3001Q1){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        opt3001q1_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_BH1721FVC:
                    if(sensor_index>=N_SENSOR_BH1721FVC){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        bh1721fvc_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_APDS9306:
                    if(sensor_index>=N_SENSOR_APDS9306){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        apds9306_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_SHT35:
                    if(sensor_index>=N_SENSOR_SHT35){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        sht35_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_SYLVATICA:
                    if(sensor_index>=N_SENSOR_SYLVATICA){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        sylvatica_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_SYLVATICA2:
                    if(sensor_index>=N_SENSOR_SYLVATICA2){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        sylvatica2_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_PLANALTA:
                    if(sensor_index>=N_SENSOR_PLANALTA){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        planalta_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_LICOR:
                    if(sensor_index>=N_SENSOR_LICOR){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        licor_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_GROWTH_CHAMBER:
                    growth_chamber_config.general.status = SENSOR_STATUS_IDLE;
                    break;
                case CAN_CONFIG_CMD_RELAY_BOARD:
                    if(sensor_index>=N_ACTUATOR_RELAY_BOARD){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        actuator_relay_board_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_PLANALTA_FS:
                    if(sensor_index>=N_SENSOR_PLANALTA){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        planalta_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                        planalta_config[sensor_index].op_mode = PLANALTA_FS;
                    }
                    break;
                case CAN_CONFIG_CMD_NAU7802:
                    if(sensor_index>=N_SENSOR_NAU7802){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        nau7802_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                case CAN_CONFIG_CMD_DS18B20:
                    if(sensor_index>=N_SENSOR_DS18B20){
                        sprintf(print_buffer, "UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                        uart_print(print_buffer, strlen(print_buffer));
                    } else {
                        ds18b20_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                    }
                    break;
                default:
                    sprintf(print_buffer, "UART: sensor/actuator type `%x` not supported.", m.data[0]);
                    uart_print(print_buffer, strlen(print_buffer));
                    break;
            }
            break;*/
        default:
            status = process_actuator_serial_command(&m);

            // TODO process_sensor_serial_command(&m);
            if(!status){
                sprintf(print_buffer, "UART: serial command `%x` not supported.", m.command);
                print_error(print_buffer, strlen(print_buffer));
            }
            break;
    }
    m.status = UART_MSG_NONE;
    
    n_uart_rx_messages--;
}
