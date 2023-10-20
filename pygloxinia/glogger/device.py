import serial.tools.list_ports as list_ports
import serial


class Device():
    def __init__(self, dev_id=None, serial_number=None):
        if dev_id is None:
            ports = list_ports.comports()
            serial_numbers = [i.serial_number for i in ports]
            print(serial_numbers)
            idx = serial_numbers.index(serial_number)
            if idx is not None:
                dev_id = ports[idx].device
            else:
                raise ValueError("Unable to find UART serial.")
        self.ser = serial.Serial(dev_id, 50000, stopbits=1, rtscts=False)
