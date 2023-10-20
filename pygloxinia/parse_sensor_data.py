import os

from glogger.gcontrol import GControl, GMessage
from glogger.device import Device
import pickle
from datetime import datetime
import serial.tools.list_ports as list_ports
import numpy as np

def decode_sensor_data(sample : GMessage):
    interface_id = (sample.extended_id & 0xff) >> 4
    sensor_id = (sample.extended_id & 0xf)
    cmd = (sample.extended_id >> 8) & 0xff
    # process only sensor data messages
    if cmd != 0x12:
        return {}
    if sample.id == 0x0:
        if interface_id == 0:
            if sensor_id == 0:
                temp = (sample.data[0] << 8) | sample.data[1]
                rh = (sample.data[3] << 8) | sample.data[4]

                temp = -45.0 + 175*(temp) / (2**16-1)
                rh = 100.0*rh / (2**16 - 1)
                return {"temp": temp, "rh": rh}
            if sensor_id == 1:
                return {"light": (sample.data[0] << 16) | (sample.data[1] << 8) | sample.data[2]}
        if (interface_id == 1) or (interface_id == 2) or (interface_id == 3):
            ch = int(np.log2(sample.data[0]))
            if (sample.data[0] % 2) == 0:
                label = f"sap.{interface_id}.ch{ch // 2}"
                value = (sample.data[1] << 16) | (sample.data[2] << 8) | sample.data[3]
            else:
                label = f"sap.{interface_id}.cal{ch // 2}"
                value = (sample.data[1] << 16) | (sample.data[2] << 8) | sample.data[3]
            return {label: value}
    if sample.id == 0xFD:
        ch = int(np.log2(sample.data[0]))
        if (interface_id == 0) or (interface_id == 1):
            if (sample.data[0] % 2) == 0:
                label = f"sap.{interface_id}.ch{ch // 2}"
                value = (sample.data[1] << 16) | (sample.data[2] << 8) | sample.data[3]
            else:
                label = f"sap.{interface_id}.cal{ch // 2}"
                value = (sample.data[1] << 16) | (sample.data[2] << 8) | sample.data[3]
            return {label: value}

import os
import pandas as pd
def parse_data(file_dir, output_file="parse_data.csv"):
    files = os.listdir(file_dir)
    parsed_data = {}
    for f in files:
        print(f"Processing {f}")

        data = pickle.load(open(os.path.join(file_dir, f), "rb"))
        for i in data:
            sample_data = decode_sensor_data(i[1])
            print(sample_data)


if __name__ == "__main__":
    parse_data(file_dir="data/")