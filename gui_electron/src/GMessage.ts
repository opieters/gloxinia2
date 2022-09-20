const luxon = require("luxon");
const DateTime = luxon.DateTime;

const start_byte = 0x01;
const stop_byte = 0x04;

export const command_types = [
    {name: "start_measurement",      code: 0x00},
    {name: "stop_measurement",       code: 0x01},
    {name: "activate_sensor",        code: 0x02},
    {name: "deactivate_sensor",      code: 0x03},
    {name: "reset_node",             code: 0x04},
    {name: "reset_system",           code: 0x05},
    {name: "text_message",           code: 0x06},
    {name: "sensor_data",            code: 0x07},
    {name: "sensor_status",          code: 0x08},
    {name: "measurement_period",     code: 0x09},
    {name: "error_message",          code: 0x0A},
    {name: "loopback_message",       code: 0x0B},
    {name: "actuator_status",        code: 0x0C},
    {name: "hello_message",          code: 0x0D},
    {name: "init_sampling",          code: 0x0E},
    {name: "init_sensors",           code: 0x0F},
    {name: "sensor_error",           code: 0x10},
    {name: "lia_gain",               code: 0x11},
    {name: "unknown",                code: 0x12},
    {name: "meas_trigger",           code: 0x13},
    {name: "sensor_config",          code: 0x14},
    {name: "actuator_data",          code: 0x15},
    {name: "actuator_error",         code: 0x16},
    {name: "actuator_trigger",       code: 0x17},
    {name: "actuator_gc_temp",       code: 0x18},
    {name: "actuator_gc_rh",         code: 0x19},
    {name: "sensor_start",           code: 0x1A},
    {name: "actuator_relay",         code: 0x1B},
    {name: "sensor_actuator_enable", code: 0x1C},
    {name: "actuator_relay_now",     code: 0x1D},
    {name: "actuator_status",        code: 0x1E},
    {name: "register_sensor",        code: 0x1F},
    {name: "config_sensor",          code: 0x20},
    {name: "readout_sensor_config",  code: 0x21}
];

export interface SerialMessage {
    readonly data: Uint8Array,
    readonly command: number,
    readonly id: number,
    readonly extended_id: number,
};


function cmd_start_measurement_handler(sm : SerialMessage){};
function cmd_stop_measurement_handler(sm : SerialMessage){};
function cmd_activate_sensor_handler(sm : SerialMessage){};
function cmd_deactivate_sensor_handler(sm : SerialMessage){};
function cmd_reset_node_handler(sm : SerialMessage){};
function cmd_reset_system_handler(sm : SerialMessage){};
function cmd_text_message_handler(sm : SerialMessage){};

var append_data_handler : (sensor_name : string, sensor_data : number) => void;

export function set_data_handler_fn(fn : (sensor_name : string, sensor_data : number) => void){
    append_data_handler = fn;
}

function cmd_sensor_data_handler(sm : SerialMessage){
    // get global and local sensor ids
    const global_id = (sm.extended_id >> 8) & 0xff;
    const local_id = sm.extended_id & 0xff;

    // find the name of the sensor type
    var ids : Array<number> = new Array<number>();
    for(var i = 0; i < sensor_types.length; i++) ids.push(sensor_types[i].code)
    var i = ids.indexOf(global_id);
    const sensor_name = `${sensor_types[i].name}.${local_id}`;
    const sensor_data = sensor_types[i].handler(sm.data);

    if(sensor_data.length > 1){
        for(var i = 0; i < sensor_data.length; i++){
            const label = `${sensor_name}.${i}`;
            append_data_handler(label, sensor_data[i]);
        }
    } else {
        append_data_handler(sensor_name, sensor_data[0]);
    }
};


function signed_value_handler(data : Uint8Array) : Array<number> { 
    var d = new Int16Array(data.length / 2);
    for(var i = 0; i < d.length; i++){
        var value = (data[2*i] << 8) | (data[2*i+1]);
        d.set([value], i);
    }
    
    return Array.from(d)
};


const sensor_types = [
    {name: "apds9301",       code: 0x01, handler: signed_value_handler},
    {name: "opt3001-q1",     code: 0x02, handler: signed_value_handler},
    {name: "bh1721fvc",      code: 0x03, handler: signed_value_handler},
    {name: "apds9306",       code: 0x04, handler: signed_value_handler},
    {name: "sht35",          code: 0x05, handler: signed_value_handler},
    {name: "sylvatica",      code: 0x06, handler: signed_value_handler},
    {name: "planalta",       code: 0x07, handler: signed_value_handler},
    {name: "licor",          code: 0x08, handler: signed_value_handler},
    {name: "growth_chamber", code: 0x09, handler: signed_value_handler},
    {name: "relay_board",    code: 0x0a, handler: signed_value_handler},
    {name: "planalta_fs",    code: 0x0b, handler: signed_value_handler},
    {name: "nau7802",        code: 0x0c, handler: signed_value_handler},
    {name: "ds18b20",        code: 0x0d, handler: signed_value_handler},
    {name: "sylvatica2",     code: 0x0e, handler: signed_value_handler}
];

function cmd_sensor_status_handler(sm : SerialMessage){};
function cmd_measurement_period_handler(sm : SerialMessage){};
function cmd_error_message_handler(sm : SerialMessage){};
function cmd_loopback_message_handler(sm : SerialMessage){};
function cmd_actuator_status_handler(sm : SerialMessage){};
function cmd_hello_message_handler(sm : SerialMessage){};
function cmd_init_sampling_handler(sm : SerialMessage){};
function cmd_init_sensors_handler(sm : SerialMessage){};
function cmd_sensor_error_handler(sm : SerialMessage){};
function cmd_lia_gain_handler(sm : SerialMessage){};
function cmd_unknown_handler(sm : SerialMessage){};
function cmd_meas_trigger_handler(sm : SerialMessage){};
function cmd_sensor_config_handler(sm : SerialMessage){};
function cmd_actuator_data_handler(sm : SerialMessage){};
function cmd_actuator_error_handler(sm : SerialMessage){};
function cmd_actuator_trigger_handler(sm : SerialMessage){};
function cmd_actuator_gc_temp_handler(sm : SerialMessage){};
function cmd_actuator_gc_rh_handler(sm : SerialMessage){};
function cmd_sensor_start_handler(sm : SerialMessage){};
function cmd_actuator_relay_handler(sm : SerialMessage){};
function cmd_sensor_actuator_enable_handler(sm : SerialMessage){};
function cmd_actuator_relay_now_handler(sm : SerialMessage){};

export function add_log_message(message: string){
    const log = <HTMLUListElement> document.getElementById('log');
    const new_entry = document.createElement("li");
    new_entry.className = "message";
    new_entry.appendChild(document.createTextNode(message));
    log.prepend(new_entry);
    if(log.childElementCount > 25){
        log.removeChild(log.childNodes[log.childElementCount - 1]);
    }
}

export function update_log_rx_message(message : SerialMessage){
    for(var i = 0; i < command_types.length; i++){
        if(command_types[i].code == message.command){
            var log_message = ""

            const timestamp = DateTime.now().toFormat('HH:mm:ss.SSS');
            if(command_types[i].code !== 0x06){
                var data = "[";
                for(var j = 0; j < message.data.length; j++){
                    data = data + " " + message.data[j].toString(16);
                }
                data = data + "]";
                log_message = `${timestamp} ${command_types[i].name} (${message.id}-${message.extended_id}): ${data}`;
            } else {
                log_message = `${timestamp} ${command_types[i].name} (${message.id}-${message.extended_id}): ${new TextDecoder().decode(message.data)}`;
            }
            add_log_message(log_message);

            return;
        }
    }
}

export function handle_received_message(sm : SerialMessage){
    update_log_rx_message(sm);
    switch(sm.command){
        case 0x00:
            cmd_start_measurement_handler(sm);
            break;
        case 0x01:
            cmd_stop_measurement_handler(sm);
            break;
        case 0x02:
            cmd_activate_sensor_handler(sm);
            break;
        case 0x03:
            cmd_deactivate_sensor_handler(sm);
            break;
        case 0x04:
            cmd_reset_node_handler(sm);
            break;
        case 0x05:
            cmd_reset_system_handler(sm);
            break;
        case 0x06:
            cmd_text_message_handler(sm);
            break;
        case 0x07: {
            cmd_sensor_data_handler(sm);
            break;
        }
        case 0x08:
            cmd_sensor_status_handler(sm);
            break;
        case 0x09:
            cmd_measurement_period_handler(sm);
            break;
        case 0x0A:
            cmd_error_message_handler(sm);
            break;
        case 0x0B:
            cmd_loopback_message_handler(sm);
            break;
        case 0x0C:
            cmd_actuator_status_handler(sm);
            break;
        case 0x0D:
            cmd_hello_message_handler(sm);
            break;
        case 0x0E:
            cmd_init_sampling_handler(sm);
            break;
        case 0x0F:
            cmd_init_sensors_handler(sm);
            break;
        case 0x10:
            cmd_sensor_error_handler(sm);
            break;
        case 0x11:
            cmd_lia_gain_handler(sm);
            break;
        case 0x12:
            cmd_unknown_handler(sm);
            break;
        case 0x13:
            cmd_meas_trigger_handler(sm);
            break;
        case 0x14:
            cmd_sensor_config_handler(sm);
            break;
        case 0x15:
            cmd_actuator_data_handler(sm);
            break;
        case 0x16:
            cmd_actuator_error_handler(sm);
            break;
        case 0x17:
            cmd_actuator_trigger_handler(sm);
            break;
        case 0x18:
            cmd_actuator_gc_temp_handler(sm);
            break;
        case 0x19:
            cmd_actuator_gc_rh_handler(sm);
            break;
        case 0x1A:
            cmd_sensor_start_handler(sm);
            break;
        case 0x1B:
            cmd_actuator_relay_handler(sm);
            break;
        case 0x1C:
            cmd_sensor_actuator_enable_handler(sm);
            break;
        case 0x1D:
            cmd_actuator_relay_now_handler(sm);
            break;
        case 0x1E:
            break;
        case 0x1F:
            break;
        default:
            console.log("Unknown command type.")
    }
}

export function sm_to_buffer(sm : SerialMessage) : Uint8Array {
    var b = new Uint8Array(7+sm.data.length)
    b.set([start_byte, sm.command, sm.id, (sm.extended_id >> 8) & 0xff, sm.extended_id & 0xff, sm.data.length], 0);
    b.set([stop_byte], b.length-1);
    b.set(sm.data, 6);
    return b;
  };

  