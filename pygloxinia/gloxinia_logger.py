from glogger.gcontrol import GControl, GMessage
from glogger.device import Device
import pickle
from datetime import datetime
import serial.tools.list_ports as list_ports


def collect_data(serial_number="auto", batch_size: int = 5000):
    if serial_number == "auto":
        ports = list_ports.comports()
        serial_numbers = [i.serial_number for i in ports]
        if len(serial_numbers) == 0:
            raise IOError("No device found.")
        dev = Device(serial_number=serial_numbers[0])
    else:
        dev = Device(serial_number=serial_number)

    control = GControl(dev=dev)

    data = []

    try:
        while 1:
            try:
                message = control.read_message(blocking=True)
                if message is None:
                    continue
                data.append((datetime.now(), message))
                print(message)
            except ValueError as e:
                print(e)
            if len(data) >= batch_size:
                pickle.dump(data, open(datetime.now().strftime("data/%Y_%m_%d-%H_%M_%S_sensor_data.pkl"), "wb"))
                data = []
    except KeyboardInterrupt:
        print("Stop experiment...")
    finally:
        print("Saving data... ", end="")
        print("{} entries".format(len(data)))
        pickle.dump(data, open(datetime.now().strftime("data/%Y_%m_%d-%H_%M_%S_sensor_data.pkl"), "wb"))
        print("Saved data")


if __name__ == "__main__":
    collect_data(serial_number="AK06R23PA")