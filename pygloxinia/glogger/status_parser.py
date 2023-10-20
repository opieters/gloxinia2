import struct
from .data_parser import name_map
from .gcontrol import GMessage

def parse_sensor_actuator_status_to_text(message : GMessage):
    if len(message.data) < 3:
        raise ValueError("Insufficient data for {}".format(message))
    global_id = message.data[0]
    local_id = message.data[1]
    status = message.data[2]

    if global_id in name_map:
        name = "{}.{}".format(name_map[global_id], local_id)
    else:
        raise ValueError("Unknown global ID {} in message {}.".format(global_id, message))

    statuses = ["inactive", "idle", "active", "error"]
    if status < len(statuses):
        state = statuses[status]
    else:
        state = "unknown"

    return "[sensor {} ({}, {}): {}]".format(name, global_id, local_id, state)

