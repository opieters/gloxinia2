import math
from collections import namedtuple

FreqConfig = namedtuple("FreqConfig", ["pwm_period", "signal_period"])


def compute_hex_values(buffer, n_duty_cycles):
    for i in range(len(buffer)):
        p_offset = 1.0
        sine_value = (1.0+math.sin(2.0*math.pi*i/len(buffer))) / 2.0
        duty_cycle_index = 0

        # find the closest PWM value
        for j in range(n_duty_cycles):
            # calculate duty cycle (not uniform!)
            if j == 0:
                duty_cycle_value = 0.0
            else:
                if j == n_duty_cycles-1:
                    duty_cycle_value = 1.0
                else:
                    duty_cycle_value = 1.0*(j+1) / n_duty_cycles

            c_offset = duty_cycle_value - sine_value
            c_offset = abs(c_offset)
            if c_offset < p_offset:
                p_offset = c_offset
                duty_cycle_index = j

        buffer[i] = duty_cycle_index


if __name__ == "__main__":
    config = {
        "50k": FreqConfig(10, 128),
        "20k": FreqConfig(16, 200),
        "10k": FreqConfig(16, 400),
        "5k": FreqConfig(16, 800),
        "2k": FreqConfig(40, 800),
        "1k": FreqConfig(80, 800),
        "500": FreqConfig(160, 800),
        "200": FreqConfig(400, 800),
        "100": FreqConfig(800, 800),
        "50": FreqConfig(1600, 800),
        "20": FreqConfig(4000, 800),
        "10": FreqConfig(8000, 800)
    }

    for k in config:
        buffer = [None]*config[k].signal_period
        compute_hex_values(buffer, config[k].pwm_period)

        buffer = [f"0x{i:04x}" for i in buffer]
        formatted_buffer = ""
        for i in range(len(buffer)):
            formatted_buffer += buffer[i]
            if i < len(buffer) - 1:
                formatted_buffer += ", "
            if i % 9 == 8:
                formatted_buffer += "\n"

        print(f"For {k}")
        print(formatted_buffer)