# Planalta

## Filter structures

| name         | DF | structure |
|:------------:|:--:|:---------:|
| F0           | 1  | BP 0.2fs - 0.3fs |
| F1           | 10 | LP 0.005fs - 0.095fs |
| F2/F3/F4/F10 | 10 | LP 0.013fs - 0.0475fs |
| F6           | 10 | LP 0.02fs - 0.045fs |
| F7           | 4  | LP 0.05fs - 0.1125fs |
| F8           | 2  | LP 0.1fs - 0.225fs |
| F9           | 8  | LP 0.025fs - 0.05625fs |

| fs   | f LIA | N | filters                                          |
|:----:|:-----:|:-:|:------------------------------------------------:|
| 200k | 50k   | 1 | F0 - MIX - F1 IQ - F2 IQ - F3 IQ - F4[0] IQ - F4[1] IQ - F8 IQ |
| 100k | 25k   | 2 | F0 - MIX - F1 IQ - F2 IQ - F3 IQ - F4 IQ - F6 IQ |
| 40k  | 10k   | 4 | F0 - MIX - F1 IQ - F2 IQ - F3 IQ - F4 IQ - F7 IQ |
| 20k  | 5k    | 8 | F0 - MIX - F1 IQ - F2 IQ - F3 IQ - F4 IQ - F8 IQ |

| fs   | f LIA | N | filters                           |
|:----:|:-----:|:-:|:---------------------------------:|
| 200k | 50k   | 1 | F0 - MIX - F1 IQ - F2 IQ - F3 IQ - F4 IQ - F10 IQ - F8 IQ |
| 80k  | 20k   | 1 | F0 - MIX - F1 IQ - F2 IQ - F3 IQ - F4 IQ - F9 IQ |
| 40k  | 10k   | 1 | F0 - MIX - F1 IQ - F2 IQ - F3 IQ - F4 IQ - F7 IQ |
| 20k  | 5k    | 1 | F0 - MIX - F1 IQ - F2 IQ - F3 IQ - F4 IQ - F8 IQ |
| 80k  | 2k    | 1 | F2 - MIX - F1 IQ - F3 IQ - F4 IQ - F9 IQ |
| 40k  | 1k    | 1 | F2 - MIX - F1 IQ - F3 IQ - F4 IQ - F7 IQ |
| 20k  | 500   | 1 | F2 - MIX - F1 IQ - F3 IQ - F4 IQ - F8 IQ |
| 80k  | 200   | 1 | F2 - F3 - MIX - F1 IQ - F4 - F9 IQ |
| 40k  | 100   | 1 | F2 - F3 - MIX - F1 IQ - F4 - F7 IQ |
| 20k  | 50    | 1 | F2 - F3 - MIX - F1 IQ - F4 - F8 IQ |
| 80k  | 20    | 1 | F2 - F3 - F4  - F0 - MIX - F1 IQ  |
| 40k  | 10    | 1 | F2 - F3 - F4  - F0 - MIX - F1 IQ |
