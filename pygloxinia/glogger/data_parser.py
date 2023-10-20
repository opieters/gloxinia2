import struct
from pygloxinia.glogger.read_data import decode_sht35, decode_apds9301
import numpy as np


__planalta_fs_list = ["50k", "20k", "10k", "5k", "2k", "1k", "500", "200", "100", "50", "20", "10"]


def parse_ds18b20(message):
    if len(message.data) == 2:
        data = struct.unpack(">h", message.data)
    else:
        raise ValueError("Data length should be 2.")

    return data


def parse_sht35(message):
    if len(message.data) == 6:
        data = struct.unpack(">HHH", message.data)
    else:
        raise ValueError("Data length should be 6.")

    return data


def parse_apds9301(message):
    if len(message.data) == 4:
        data = struct.unpack(">HH", message.data)
    else:
        raise ValueError("Data length should be 4.")
    return data


def parse_apds9306(message):
    if len(message.data) == 3:
        data = struct.unpack(">BBB", message.data)
        data = (data[0] | (data[1] << 8) | (data[2] << 16),)
    else:
        raise ValueError("Data length should be 3.")
    return data


def parse_sylvatica(message):
    length = len(message.data)
    fmt = "h" * (length // 2)
    data = struct.unpack(">" + fmt, message.data)
    return data


def parse_sylvatica2(message):
    length = len(message.data)
    fmt = "i" * (length // 4)
    data = struct.unpack(">" + fmt, message.data)
    return data


def parse_planalta(message):
    length = len(message.data)
    fmt = "h" * (length // 2)
    data = struct.unpack(">" + fmt, message.data)
    return data


def parse_bh1721fvc(message):
    if len(message.data) == 2:
        data = struct.unpack(">H", message.data)
    else:
        raise ValueError("Data length should be 2.")
    return data


def parse_licor(message):
    return []


def parse_pump(message):
    if len(message.data) == 1:
        data = message.data[0]
    else:
        raise ValueError("Pump data message should have length 1, not {}.".format(len(message.data)))
    return data


def parse_growth_chamber(message):
    raise NotImplementedError


def parse_relay_board(message):
    raise NotImplementedError


def parse_nau7802(message):
    if len(message.data) == 4:
        data = struct.unpack(">l", message.data)
    else:
        raise ValueError("Data length should be 4.")
    return data


parser_map = {
    0x01: parse_apds9301,
    0x03: parse_bh1721fvc,
    0x04: parse_apds9306,
    0x05: parse_sht35,
    0x06: parse_sylvatica,
    0x07: parse_planalta,
    0x08: parse_licor,
    0x09: parse_growth_chamber,
    0x0a: parse_relay_board,
    0x0b: parse_planalta,
    0x0c: parse_nau7802,
    0x0d: parse_ds18b20,
    0x0e: parse_sylvatica2
}

name_map = {
    0x01: "apds9301",
    0x02: "opt3001-q1",
    0x03: "bh1721fvc",
    0x04: "apds9306",
    0x05: "sht35",
    0x06: "sylvatica",
    0x07: "planalta",
    0x08: "licor",
    0x09: "growth_chamber",
    0x0a: "relay_board",
    0x0b: "planalta_fs",
    0x0c: "nau7802",
    0x0d: "ds18b20",
    0x0e: "sylvatica2"
}


key_map = {name_map[i]: i for i in name_map}


def decode_planalta(x):
    if len(x) == 16:
        x = [[x[i], x[i+1]] for i in range(0,len(x),2)]
        data = dict()
        for i in range(len(x)):
            data[f"I{i}"] = x[i][0]
            data[f"Q{i}"] = x[i][1]
        return data
    else:
        data = dict()
        for i in range(8):
            data[f"I{i}"] = np.nan
            data[f"Q{i}"] = np.nan
        return data


def decode_sylvatica(x):
    if len(x) == 8:
        data = dict()
        for i in range(len(x)):
            data[f"C{i}"] = x[i]
        return data
    else:
        data = dict()
        for i in range(len(x)):
            data[f"C{i}"] = np.nan
        return data


def decode_single_value(x):
    if len(x) == 1:
        return {"data": x[0]}
    else:
        return {"data": np.nan}


def decode_empty_value(x):
    return {"data": np.nan}


def decode_to_do(x):
    raise NotImplementedError


def decode_pass_value(x):
    return {"data": x}


def decode_planalta_fs(x):
    data = dict()
    if len(x) != (2*len(__planalta_fs_list)):
        for i in range(0, 2*len(__planalta_fs_list), 2):
            data[f"{__planalta_fs_list[i//2]}-I"] = np.nan
            data[f"{__planalta_fs_list[i//2+1]}-Q"] = np.nan
    else:
        for i in range(0, len(x), 2):
            data[f"{__planalta_fs_list[i//2]}-I"] = x[i]
            data[f"{__planalta_fs_list[i//2]}-Q"] = x[i+1]
    return data


def get_name(message):
    global_id = (message.extended_id >> 8) & 0xff
    local_id = message.extended_id & 0xff
    if global_id in name_map:
        return name_map[global_id], local_id
    else:
        raise ValueError("Global ID {} not recognised.".format(global_id))


decode_map = {
    0x01: decode_apds9301,
    0x03: decode_single_value,
    0x04: decode_single_value,
    0x05: decode_sht35,
    0x06: decode_sylvatica,
    0x07: decode_planalta,
    0x08: decode_empty_value,
    0x09: decode_pass_value,
    0x0a: decode_to_do,
    0x0b: decode_planalta_fs,
    0x0c: decode_single_value,
    0x0d: decode_single_value,
    0x0e: decode_sylvatica,
}


def decode(message, data):
    sensor_type = (message.extended_id >> 8) & 0xff
    if sensor_type not in parser_map:
        raise ValueError("No parser available for this sensor type.")
    return decode_map[sensor_type](data)


def parse(message):
    sensor_type = (message.extended_id >> 8) & 0xff
    if sensor_type not in parser_map:
        raise ValueError("No parser available for this sensor type.")
    return parser_map[sensor_type](message)
