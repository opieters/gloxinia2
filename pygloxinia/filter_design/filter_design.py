import scipy.signal as signal
import numpy as np
import matplotlib.pyplot as plt
import os
from copy import deepcopy

def filter_zeros(coeffs):
    i = 0
    while (i < len(coeffs)) and (coeffs[i] == 0):
        i+=1
    coeffs = coeffs[i:]
    i = len(coeffs)
    while (i > 0) and (coeffs[i-1] == 0):
        i -= 1
    coeffs = coeffs[:i]

    return coeffs


def fractional(x, format=15):
    x *= 2**format
    x = np.round(x)
    x[x < -2**format] = -2**format
    x[x > 2**format - 1] = 2**format - 1
    x = x / (2**format)

    return x


def fractional_to_hex(x, format=15, normalise=True, verbose=False, gain=1.0):
    correction_factor = 1.0
    h = np.array([2**16])*gain
    if normalise:
        while (sum(h) / 2**15) > gain:
            h = np.copy(x / correction_factor)
            h *= (2**format)
            h = np.round(h)
            # avoid large jumps
            h[h > (2**format - 1)] = 2**format - 1
            h[h < -2**format] = -2**format

            if verbose:
                print("Gain after conversion:", sum(h) / 2**15)

            correction_factor += 0.001
    else:
        h = np.copy(x)
        h *= (2 ** format)
        h = np.round(h)
        # avoid large jumps
        h[h > (2 ** format - 1)] = 2 ** format - 1
        h[h < -2 ** format] = -2 ** format

    h = h.astype(np.int16).astype(np.uint16)
    return h


def generate_c_code(x, format=15):
    x = fractional(x, format=format)
    x = fractional_to_hex(x, format=format)

    code = ""
    for i in range(len(x)):
        code += "0x%04x" % x[i]
        if i % 4 == 3:
            code += "\n"
        elif i != (len(x) - 1):
            code += ", "

    return code


def save_to_asm_file(filters, filename, init_struct=True, init_delay=True, new_name=False):
    filters = deepcopy(filters)
    space = ""  # ""            "

    with open(filename, "w") as f:
        for n, fir in enumerate(filters):

            if (n % 2 == 0) and (not fir["split"]) and fir["iq_filter"]:
                continue

            if fir["split"]:
                if n % 2 == 0:
                    fir["coeffs_hex"] = fir["coeffs_hex"][::2]
                else:
                    fir["coeffs_hex"] = fir["coeffs_hex"][1::2]

            if fir["split"]:
                if n % 2 == 0:
                    name = "fir_coeffs_{}_i".format(fir["index"])
                else:
                    name = "fir_coeffs_{}_q".format(fir["index"])
                align = 2 ** (int(np.ceil(np.log2(len(fir["coeffs_hex"])))))
            else:
                name = "fir_coeffs_{}".format(fir["index"])
                align = 2 ** (int(np.ceil(np.log2(2 * len(fir["coeffs_hex"])))))

            if name is not None:
                f.write("; number of filter taps\n")
                f.write(".equ %s_ntaps, %d\n\n" % (name, len(fir["coeffs_hex"])))
                f.write(space + ".section ybss, ymemory, eds, data\n")  # f.write(space+".section .prog\n")
                f.write(space+".align %d\n" % align)
                f.write("\n")

                if not new_name:
                    f.write(name + "_taps:\n")
                else:
                    label = "_" + name
                    f.write(".global %s\n" % label)
                    f.write(label + ":\n")
            for i in range(len(fir["coeffs_hex"])):
                if i % 9 == 0:
                    if i != 0:
                        f.write("\n")
                    f.write(".hword ")
                f.write("0x%04x" % fir["coeffs_hex"][i])
                if i != len(fir["coeffs_hex"])-1 and i % 9 != 8:
                    f.write(", ")
            f.write("\n")

            if init_delay:
                f.write("\n; allocate uninitialised y data space delay buffer\n")
                f.write(".section xbss, xmemory, eds, data\n")
                #f.write("%s_delay:\n" % name)
                f.write(".align %d\n\n" % align)
                f.write(".global _%s_delay\n" % name)
                f.write("_%s_delay:\n" % name)
                f.write(".space %s_ntaps*2\n" % name)

            if init_struct:
                f.write("; initialise the filter structure\n")
                f.write(".section data\n")
                f.write(".global _%s\n" % name)
                f.write("_%s:\n" % name)
                f.write(".hword %s_ntaps\n" % name)
                f.write(".hword %s_taps\n" % name)
                f.write(".hword %s_taps+%s_ntaps*2-1\n" % (name, name))
                f.write(".hword 0xff00\n")
                f.write(".hword _%s_delay\n" % name)
                f.write(".hword _%s_delay+%s_ntaps*2-1\n" % (name, name))
                f.write(".hword _%s_delay\n" % name)

            f.write("\n\n")


def design_planalta():
    asm_dir = "planalta"

    filter_type1 = {
        "fs": 20e3,
        "numtaps": 24,
        "bands":   [0, 2e3, 4e3, 6e3, 8e3, 10e3],
        "desired": [0, 0,       1,   1,     0,    0],
        "bandpass": True,
        "correction_factor": 1.05,
        "split": False,
        "iq_filter": False,
        "decimation": 1,
        "n_copies": "PLANALTA_5KHZ_N_ADC_CHANNELS"
        }
    filter_type2 = {
        "fs": 20e3,
        "numtaps": 46,
        "bands": [0, 100, 1.9e3, 10e3],
        "desired": [1, 1, 0, 0],
        "gain": 2.0,
        "bandpass": False,
        "split": True,
        "iq_filter": True,
        "decimation": 10,
        "n_copies": "PLANALTA_5KHZ_N_ADC_CHANNELS"
        }
    filter_type3 = {
        "fs": 2e3,
        "numtaps": 96,
        "bands": [0, 26, 95, 1e3],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "split": False,
        "iq_filter": True,
        "decimation": 10,
        "n_copies": "PLANALTA_5KHZ_N_ADC_CHANNELS"
        }
    filter_type4 = {
        "fs": 20,
        "numtaps": 380,
        "bands": [0, 0.2, 0.45, 10],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "split": False,
        "iq_filter": True,
        "decimation": 20,
        "n_copies": "PLANALTA_50KHZ_N_ADC_CHANNELS"
        }
    filter_type5 = {
        "fs": 10,
        "numtaps": 200,
        "bands": [0, 0.2, 0.45, 5],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "split": False,
        "iq_filter": True,
        "decimation": 10,
        "n_copies": "PLANALTA_25KHZ_N_ADC_CHANNELS"
        }
    filter_type6 = {
        "fs": 4,
        "numtaps": 80,
        "bands": [0, 0.2, 0.45, 2],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "split": False,
        "iq_filter": True,
        "decimation": 4,
        "n_copies": "PLANALTA_10KHZ_N_ADC_CHANNELS"
    }
    filter_type7 = {
        "fs": 2,
        "numtaps": 40,
        "bands": [0, 0.2, 0.45, 1],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "split": False,
        "iq_filter": True,
        "decimation": 2,
        "n_copies": "PLANALTA_5KHZ_N_ADC_CHANNELS"
    }
    filter_type8 = {
        "fs": 8,
        "numtaps": 8*20,
        "bands": [0, 0.2, 0.45, 4],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "split": False,
        "iq_filter": True,
        "decimation": 8,
        "n_copies": "PLANALTA_50KHZ_N_ADC_CHANNELS"
    }
    filter_type9 = {
        "fs": 80e3,
        "numtaps": 200,
        "bands": [0, 2.1e3, 4e3, 40e3],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "split": False,
        "iq_filter": False,
        "decimation": 10,
        "n_copies": "PLANALTA_50KHZ_N_ADC_CHANNELS"
    }
    filter_type10 = {
        "fs": 8,
        "numtaps": 8 * 20,
        "bands": [0, 0.2, 0.45, 4],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "split": True,
        "gain": 2.0,
        "iq_filter": True,
        "decimation": 8,
        "n_copies": "PLANALTA_50KHZ_N_ADC_CHANNELS"
    }
    filter_type11 = {
        "fs": 4,
        "numtaps": 4 * 20,
        "bands": [0, 0.2, 0.45, 2],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "gain": 2.0,
        "split": True,
        "iq_filter": True,
        "decimation": 4,
        "n_copies": "PLANALTA_10KHZ_N_ADC_CHANNELS"
    }

    filters = [
        deepcopy(filter_type1),  # 0
        deepcopy(filter_type2),  # 1
        deepcopy(filter_type3),  # 2
        deepcopy(filter_type3),  # 3
        deepcopy(filter_type3),  # 4
        deepcopy(filter_type4),  # 5
        deepcopy(filter_type5),  # 6
        deepcopy(filter_type6),  # 7
        deepcopy(filter_type7),  # 8
        deepcopy(filter_type8),  # 9
        deepcopy(filter_type9),  # 10
        deepcopy(filter_type10),
        deepcopy(filter_type11)
    ]

    filters = convert_filters(filters=filters)

    save_to_asm_file(filters, os.path.join(asm_dir, "fir_coeffs.s"), init_struct=False,
                     init_delay=False, new_name=True)

    for n, fir in enumerate(filters):
        if n % 2 == 0:
            plot_fir(fir)


def design_sylvatica():
    asm_dir = "sylvatica"

    filter_type1 = {
        "fs": 10e3,
        "numtaps": 102,
        "bands": [0, 20, 500, 5e3],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "split": False,
        "iq_filter": False,
        "decimation": 10,
        "n_copies": "SYLVATICA_N_CHANNELS"
        }
    filter_type2 = {
        "fs": 10,
        "numtaps": 200,
        "bands": [0, 0.2, 0.5, 5],
        "desired": [1, 1, 0, 0],
        "bandpass": False,
        "split": False,
        "iq_filter": False,
        "decimation": 10,
        "n_copies": "SYLVATICA_N_CHANNELS"
    }


    filters = [
        deepcopy(filter_type1),
        deepcopy(filter_type1),
        deepcopy(filter_type1),
        deepcopy(filter_type2),
    ]

    filters = convert_filters(filters=filters)

    save_to_asm_file(filters, os.path.join(asm_dir, "fir_coeffs.s"), init_struct=False,
                     init_delay=False, new_name=True)

    save_to_c_file(filters, filename=os.path.join(asm_dir, "fir_coeffs.c"))
    save_to_h_file(filters, filename=os.path.join(asm_dir, "fir_coeffs.h"))

    save_filters_c_file(filters, os.path.join(asm_dir, "filters_sylvatica.c"), "filters_sylvatica.h", "sylvatica.h")

    save_filters_h_file(filters = filters,
                        filename=os.path.join(asm_dir, "filters_sylvatica.h"),
                        #n_channels_macro="PLANALTA_N_ADC_CHANNELS",
                        project_name="SYLVATICA",
                        #n_channels=8
                        )

    for n, fir in enumerate(filters):
        plot_fir(fir)


def plot_fir(fir):
    freqs = np.logspace(np.log10(2 * np.pi * min(fir["bands"][1:]) / 10 / (fir["fs"])), np.log10(np.pi), 2 ** 10)
    freqs = np.linspace(2 * np.pi * min(fir["bands"][1:]) / 10 / (fir["fs"]), np.pi, 2 ** 10)
    freqs[0] = 0
    w, h = signal.freqz(fir["coeffs"], worN=freqs)
    plt.figure()
    plt.title("filter {} ({})".format(fir["numtaps"], len(fir["coeffs"])))
    plt.plot(fir["fs"] * w / (2 * np.pi), 10 * np.log10(np.abs(h)))
    plt.ylim([-55, 5])
    xlims = plt.xlim()
    ylims = plt.ylim()
    plt.plot(fir["bands"], [10*np.log10(j) if (j > 0) else ylims[0] for j in fir["desired"]], "--r")
    plt.xlim(xlims)
    plt.ylim(ylims)
    plt.xlabel("frequency [Hz]")
    plt.ylabel("amplitude [dB]")
    plt.grid()
    plt.show()


def convert_filters(filters, verbose=False):
    for n, fir in enumerate(filters):
        fir["coeffs"] = signal.remez(
                numtaps=fir["numtaps"],
                bands=fir["bands"],
                desired=fir["desired"][::2],
                fs=fir["fs"])

        if "correction_factor" in fir:
            fir["coeffs"] /= fir["correction_factor"]

        if not fir["bandpass"]:
            if verbose:
                print("Gain:", np.sum(fir["coeffs"]))
            fir["coeffs"] /= np.sum(fir["coeffs"])
            if verbose:
                print("Corrected gain:", np.sum(fir["coeffs"]))

        if "gain" not in fir:
            fir["gain"] = 1.0
        fir["coeffs"] *= fir["gain"]

        fir["coeffs_hex"] = fractional_to_hex(fir["coeffs"], normalise=not fir['bandpass'], gain=fir["gain"])

        fir["coeffs_hex"] = filter_zeros(fir["coeffs_hex"])

        fir["index"] = n

        print(n)
        print([i["index"] for i in filters if "index" in i])
        filters[n] = fir

    print([i["index"] for i in filters if "index" in i])

    insert_filters = []
    for n, fir in enumerate(filters):
        if fir["split"] or fir["iq_filter"]:
            insert_filters.append((n + 1 + len(insert_filters), deepcopy(fir)))

    print([i["index"] for i in filters])

    for i in insert_filters:
        filters.insert(i[0], i[1])

    print([i["index"] for i in filters])

    return filters







if __name__ == "__main__":
    #design_sylvatica()
    design_planalta()
