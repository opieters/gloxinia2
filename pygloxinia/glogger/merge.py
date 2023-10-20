import os
import numpy as np
import pandas as pd
from pygloxinia.glogger.read_data import load_data_v3

name_map = {
    "time": "time",
    "apds9306.0": "light_sensor_lux",
    "sylvatica.0.C1": "leaf_temp_3_nc_au",
    "sylvatica.0.C2": "leaf_thickness_7_nc_au",
    "sylvatica.0.C3": "leaf_temp_7_nc_au",
    "sylvatica.0.C4": "leaf_thickness_4_nc_au",
    "sylvatica.0.C5": "leaf_temp_4_nc_au",
    "sylvatica.0.C6": "ref_mon_0",
    "sylvatica.0.C7": "leaf_thickness_3_nc_au",

    "sylvatica.1.C1": "leaf_temp_2_nc_au",
    "sylvatica.1.C3": "leaf_thickness_2_nc_au",
    "sylvatica.1.C6": "ref_mon_1",

    "sylvatica.2.C1": "ref_mon_2",
    "sylvatica.2.C3": "soil_water_content_nc_au",
    "sylvatica.2.C6": "leaf_temp_6_nc_au",
    "sylvatica.2.C7": "leaf_thickness_6_nc_au",

    "sylvatica.3.C1": "ref_mon_3",
    "sylvatica.3.C2": "leaf_thickness_1_nc_au",
    "sylvatica.3.C3": "leaf_temp_1_nc_au",
    "sylvatica.3.C4": "leaf_thickness_8_nc_au",
    "sylvatica.3.C5": "leaf_temp_8_nc_au",
    "sylvatica.3.C6": "leaf_thickness_5_nc_au",
    "sylvatica.3.C7": "leaf_temp_5_nc_au",

    "sylvatica.3.C2_calibrated": "leaf_thickness_1_um",
    "sylvatica.1.C3_calibrated": "leaf_thickness_2_um",
    "sylvatica.0.C7_calibrated": "leaf_thickness_3_um",
    "sylvatica.0.C4_calibrated": "leaf_thickness_4_um",
    "sylvatica.3.C6_calibrated": "leaf_thickness_5_um",
    "sylvatica.2.C7_calibrated": "leaf_thickness_6_um",
    "sylvatica.0.C2_calibrated": "leaf_thickness_7_um",
    "sylvatica.3.C4_calibrated": "leaf_thickness_8_um",
    "sylvatica.2.C3_calibrated": "soil_water_content_au",
    "sht35.0.temp": "air_temperature_C",
    "sht35.0.rh": "relative_humidity_percent",
    "xTair": "li6400xt_external_probe_air_temperature_C",
    "xRH": "li6400xt_external_probe_relative_humidity_percent",
    "Cond": "li6400xt_stomatal_conductance_mol/m2/s",
    "Photo": "li6400xt_photosynthetic_rate_umol/m2/s",
    "Trmmol": "li6400xt_transpiration_rate_mmol/m2/s",
    "VpdL": "li6400xt_vapour_pressure_deficit_kPa",
    "Tair": "li6400xt_sample_cell_air_temperature_C",
    "Tleaf": "li6400xt_leaf_temperature_C",
    "CO2R": "li6400xt_ref_cell_CO2_conc_umol/mol",
    "CO2S": "li6400xt_sample_cell_CO2_conc_umol/mol",
    "H2OR": "li6400xt_ref_cell_H2O_conc_mmol/mol",
    "H2OS": "li6400xt_sample_cell_H2O_conc_mmol/mol",
    "RH_R": "li6400xt_ref_cell_relative_humidity_conc_percent",
    "RH_S": "li6400xt_sample_cell_relative_humidity_conc_percent",
    "PARi": "li6400xt_PAR_inside_chamber_umol/m2/s",
    "PARo": "li6400xt_PAR_outside_chamber_umol/m2/s",
    "Press": "li6400xt_air_pressure_kPa",
    # "CTleaf": "li6400xt_computed_leaf_temperature_C",
    # "Ci": "li6400xt_intercellular_CO2_conc_umol/ms/s"
    "train_val_test_split": "train_val_test_split",
    "train_val_test_split2": "train_val_test_split2",
}


def process_df(data_dir: str) -> pd.DataFrame:
    files = os.listdir(data_dir)
    env_files = [i for i in files if ".pkl" in i]
    env_files = sorted(env_files)

    df = dict()
    n_samples = 0
    for i in range(0, len(env_files), 20):
        data = load_data_v3(data_dir, env_files[i:i + 20])

        for k in data:
            if k in df:
                df[k] += data[k]
            else:
                if n_samples > 0:
                    df[k] = data[k]
                else:
                    df[k] = [None] * n_samples + data[k]
        # check if all vectors have the same length
        max_length = np.max([len(df[k]) for k in df])
        for k in df:
            if len(df[k]) < max_length:
                d_length = max_length - len(df[k])
                df[k] = df[k] + [None] * d_length
        print(".", end="")

    # fix all None values
    keys = list(df.keys())
    if "licor.0" in keys:
        keys.remove('licor.0')
    if "time" in keys:
        keys.remove('time')
    for k in keys:
        v = df[k]
        max_length = None
        for i in v:
            if isinstance(i, tuple):
                max_length = len(i)
                break
        if max_length is None:
            continue
        for i in range(len(df[k])):
            if (df[k][i] is None) or (len(df[k][i]) < max_length):
                df[k][i] = (np.nan,) * max_length

    rm_keys = []
    dfn = dict()
    for k in rm_keys:
        del df[k]

    for k in dfn:
        df[k] = dfn[k]
    del dfn

    if "licor.0" in df:
        mask = [i is not None for i in df["licor.0"]]
        df["licor.0"] = mask

    df = pd.DataFrame(df)

    return df


def rename_columns(df: pd.DataFrame, new_names:dict = name_map):
    keys = list(df.keys())
    remove_keys = [i for i in keys if i not in new_names]
    remove_keys = [i for i in remove_keys if not i.startswith("di_")]
    remove_keys = [i for i in remove_keys if not i.startswith("di2_")]
    df.drop(columns=remove_keys, inplace=True)
    df.rename(columns=new_names, inplace=True)

    return df

