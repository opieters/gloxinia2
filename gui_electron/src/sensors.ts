const { SerialPort } = require('serialport');

const { handle_received_message, SerialMessage, set_data_handler_fn, update_log_rx_message, sm_to_buffer, add_log_message } = require("./GMessage")

function initSensorAPDS9301(port : typeof SerialPort, id : number, extended_id: number){
    // register sensor in the device
    var message : typeof SerialMessage;

    // TODO
    message = {
        data:[], 
        command: "register_sensor",
        id: id,
        extended_id: extended_id
    };
    port.write(sm_to_buffer(message));

    // 0: wake sensor
    message = {
        data:[0b10000000, 0x03], 
        command: "config_sensor",
        id: id,
        extended_id: extended_id
    };
    port.write(sm_to_buffer(message));

    // 1: set the timing register
    message = {
        data:[0b10000001, 0b00000000 | 0b00000010], 
        command: "config_sensor",
        id: id,
        extended_id: extended_id
    };
    port.write(sm_to_buffer(message));

    // 2: address of LSB of lower threshold
    message = {
        data: [0b10100010, 0, 0], 
        command: "config_sensor",
        id: id,
        extended_id: extended_id
    };
    port.write(sm_to_buffer(message));

    // 3: address of LSB of upper threshold
    message = {
        data: [0b10100100, 0, 0], 
        command: "config_sensor",
        id: id,
        extended_id: extended_id
    };
    port.write(sm_to_buffer(message));

    // 4: interrupt control register
    message = {
        data: [0b10000110, 0b00000000 | 0b00000000], 
        command: "config_sensor",
        id: id,
        extended_id: extended_id
    };
    port.write(sm_to_buffer(message));

    // set sensor readout process TODO
    message = {
        data: [0x0C | 0x80 | 0x20, ], 
        command: "readout_config_sensor",
        id: id,
        extended_id: extended_id
    };
    port.write(sm_to_buffer(message));

}