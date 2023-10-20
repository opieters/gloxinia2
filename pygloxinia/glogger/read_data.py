import os
import pickle
import numpy as np
import datetime
import pandas as pd

# these functions load the data from file and convert them into a practical format


def decode_sht35(x):
    b0 = np.array(x[0::3], dtype=np.uint16)
    b1 = np.array(x[1::3], dtype=np.uint16)
    b2 = np.array(x[2::3], dtype=np.uint16)

    temp = -45 + 175 * b0.astype(np.float32) / (2 ** 16 - 1)
    rh = ((b1 & 0xff) << 8) | ((b2 >> 8) & 0xff)
    rh = 100 * rh.astype(np.float32) / (2 ** 16 - 1)
    return {"temp": temp, "rh": rh}


def decode_apds9306(x):
    return {"pd1": np.array(x[::2]), "pd2": np.array(x[1::2])}



def load_data(data_dir, env_files=None):
    files = os.listdir(data_dir)
    if env_files is None:
        env_files = [i for i in files if ".pkl" in i]
        env_files = sorted(env_files)
    env_data_raw = [pickle.load(open(os.path.join(data_dir, i), "rb")) for i in env_files]
    env_data = dict()
    for i in range(len(env_data_raw)):
        for k in env_data_raw[i]:
            if k not in env_data:
                env_data[k] = env_data_raw[i][k]
            else:
                env_data[k] += env_data_raw[i][k]

    data = {**decode_sht35(env_data[0x0]), **env_data}
    data = {**data, **decode_apds9301(env_data[0x100])}
    fix_jumps(data, keys=["temp", "rh"])
    del data[0x0]
    del data[0x100]
    for i in range(8):
        if 0x600 + i in env_data:
            data["syl_channel_{}".format(i)] = env_data[0x600 + i]
        if 0x600 + i + 16 in env_data:
            data["syl_channel_{}".format(i + 8)] = env_data[0x600 + i + 16]
    for i in range(8):
        if 0x600 + i + 8 in env_data:
            data["syl_raw_{}".format(i)] = env_data[0x600 + i + 8]
        if 0x600 + i + 16 in env_data:
            data["syl_raw_{}".format(i + 8)] = env_data[0x600 + i + 16 + 8]
    for i in range(8):
        if 0x700 + i in env_data:
            data["pla_channel_{}".format(i)] = env_data[0x700 + i]
    del env_data

    for i in range(8):
        if "pla_channel_{}".format(i) not in data:
            continue
        if i % 2 == 0:
            data["pla_sample_{}_i".format(i // 2)] = data["pla_channel_{}".format(i)][::2]
            data["pla_sample_{}_q".format(i // 2)] = data["pla_channel_{}".format(i)][1::2]

            data["pla_sample_{}_amp".format(i // 2)] = np.sqrt(
                np.power(data["pla_sample_{}_i".format(i // 2)], 2.0)
                + np.power(data["pla_sample_{}_q".format(i // 2)], 2.0))

            data["pla_sample_{}_ang".format(i // 2)] = np.arctan2(
                np.power(data["pla_sample_{}_q".format(i // 2)], 2.0),
                np.power(data["pla_sample_{}_i".format(i // 2)], 2.0))
        else:
            data["pla_ref_{}_i".format(i // 2)] = data["pla_channel_{}".format(i)][::2]
            data["pla_ref_{}_q".format(i // 2)] = data["pla_channel_{}".format(i)][1::2]

            data["pla_ref_{}_amp".format(i // 2)] = np.sqrt(
                np.power(data["pla_ref_{}_i".format(i // 2)], 2.0)
                + np.power(data["pla_ref_{}_q".format(i // 2)], 2.0))

            data["pla_ref_{}_ang".format(i // 2)] = np.arctan2(
                np.power(data["pla_ref_{}_q".format(i // 2)], 2.0),
                np.power(data["pla_ref_{}_i".format(i // 2)], 2.0))

        del data["pla_channel_{}".format(i)]

    for i in data:
        data[i] = np.array(data[i])
    return data


def load_data_v2(data_dir, env_files=None):
    files = os.listdir(data_dir)
    if env_files is None:
        env_files = [i for i in files if ".pkl" in i]
        env_files = sorted(env_files)
        env_files = env_files
    env_data_raw = [pickle.load(open(os.path.join(data_dir, i), "rb")) for i in env_files]
    env_data = list()
    batch_size = 5000
    # convert to list of dicts
    for i in range(len(env_data_raw)):
        env_data.append(dict())
        n_samples = 0
        for sample in env_data_raw[i]:
            # add additional entries if there were more entries than expected.
            if n_samples > batch_size:
                for k in env_data[i]:
                    env_data[i][k].append(None)
            for k in sample:
                if k not in env_data[i]:
                    env_data[i][k] = [None] * batch_size
                    env_data[i][k][n_samples] = sample[k]
                else:
                    env_data[i][k][n_samples] = sample[k]
            n_samples += 1
        for k in env_data[-1]:
            env_data[-1][k] = env_data[-1][k][:n_samples]

    # create possible missing keys
    keys = []
    for i in range(len(env_data)):
        keys += list(env_data[i].keys())
    keys = list(set(keys))
    for k in keys:
        for i in range(len(env_data)):
            local_keys = list(env_data[i].keys())
            if len(local_keys) == 0:
                continue
            if k not in local_keys:
                env_data[i][k] = [None] * len(env_data[i][local_keys[0]])
    env_data_raw = dict()
    for i in range(len(env_data)):
        for k in env_data[i]:
            if k not in env_data_raw:
                env_data_raw[k] = env_data[i][k]
            else:
                env_data_raw[k] += env_data[i][k]

    return env_data_raw


def load_data_v3(data_dir, env_files=None):
    files = os.listdir(data_dir)
    if env_files is None:
        env_files = [i for i in files if ".pkl" in i]
        env_files = sorted(env_files)
        env_files = env_files
    env_data_raw = [pickle.load(open(os.path.join(data_dir, i), "rb")) for i in env_files]
    env_data = list()
    batch_size = 5000
    # convert to list of dicts
    for i in range(len(env_data_raw)):
        env_data.append(dict())
        n_samples = 0
        for sample in env_data_raw[i]:
            # add additional entries if there were more entries than expected.
            if n_samples > batch_size:
                for k in env_data[i]:
                    env_data[i][k].append(None)
            for k in sample:
                if k == "start":
                    continue
                if isinstance(sample[k], np.ndarray) and (len(sample[k]) == 1):
                    sample[k] = sample[k][0]
                if k not in env_data[i]:
                    env_data[i][k] = [None] * batch_size
                    env_data[i][k][n_samples] = sample[k]
                else:
                    if n_samples < len(env_data[i][k]):
                        env_data[i][k][n_samples] = sample[k]
                    else:
                        env_data[i][k].append(sample[k])
            n_samples += 1
        for k in env_data[-1]:
            env_data[-1][k] = env_data[-1][k][:n_samples]

    # create possible missing keys
    keys = []
    for i in range(len(env_data)):
        keys += list(env_data[i].keys())
    keys = list(set(keys))
    for k in keys:
        for i in range(len(env_data)):
            local_keys = list(env_data[i].keys())
            if len(local_keys) == 0:
                continue
            if k not in local_keys:
                env_data[i][k] = [None] * len(env_data[i][local_keys[0]])
    env_data_raw = dict()
    for i in range(len(env_data)):
        for k in env_data[i]:
            if k not in env_data_raw:
                env_data_raw[k] = env_data[i][k]
            else:
                env_data_raw[k] += env_data[i][k]

    return env_data_raw


