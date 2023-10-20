from collections import namedtuple

GCommand = namedtuple('GCommand', ["name", "number", "len"])


class GMessage:
    cmd_start = 0x5B
    cmd_stop  = 0x5D
    raw       = 0x1b

    def __init__(self, id: int, extended_id, data, rqst):
        if isinstance(data, int):
            data = bytes([data])
        elif isinstance(data, list):
            if len(data) > 0:
                if isinstance(data[0], int):
                    data = bytes(data)
            else:
                data = bytes([])

        self.data = data
        self.id = id
        self.extended_id = extended_id
        self.rqst = rqst

    def to_bytes(self):
        i = bytes([GMessage.cmd_start])
        i += bytes([self.id >> 8])
        i += bytes([self.id & 0xff])
        i += bytes([int(self.rqst)])
        i += bytes([self.extended_id >> 16, (self.extended_id >> 8) & 0xff, self.extended_id & 0xff])
        if self.data is not None:
            i = i + bytes([len(self.data)]) + self.data + bytes([GMessage.cmd_stop])
        else:
            i = i + bytes([0]) + bytes([GMessage.cmd_stop])
        return i

    def __repr__(self):
        data = ["0x%02x" % i for i in self.data]
        return str(data)
        #return "[%s: %s]" % (GMessage.commands[self.command.number], data)

    def __str__(self):
        if self.data is not None:
            data = ["0x%02x" % i for i in self.data]
        else:
            data = []
        return "[(%x, %x, %s): %s]" % (self.id, self.extended_id, "RQST" if self.rqst else "STD", data)


class GControl:
    __start_byte__ = bytes([GMessage.cmd_start])
    __stop_byte__ = bytes([GMessage.cmd_stop])

    def __init__(self, dev):
        self.dev = dev
        self.ser = dev.ser

    def write_message(self, message: GMessage):
        self.ser.write(message.to_bytes())

    def read_message(self, blocking=True) -> GMessage:
        if blocking:
            cmd = b""
            while (cmd != GControl.__start_byte__) and blocking:
                cmd = self.ser.read(1)
        else:
            if self.ser.in_waiting:
                cmd = self.ser.read(1)
                if cmd != GControl.__start_byte__:
                    return None
            else:
                return None

        id_high = self.ser.read(1)[0]
        id_low = self.ser.read(1)[0]
        id = (id_high << 8) | id_low
        rqst = self.ser.read(1)[0] > 0
        extended_id_upper = self.ser.read(1)[0]
        extended_id_high = self.ser.read(1)[0]
        extended_id_low = self.ser.read(1)[0]
        extended_id = (extended_id_upper << 16) | (extended_id_high << 8) | extended_id_low
        length = self.ser.read(1)[0]
        data = self.ser.read(length)
        stop = self.ser.read(1)

        if stop != GControl.__stop_byte__:
            raise ValueError("Incorrectly formatted message of length %d" % length + ".")
        else:
            return GMessage(id=id, extended_id=extended_id, data=data, rqst=rqst)

    def clean(self):
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()
