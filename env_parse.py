import numpy as np
import argparse
import os

########################## USER PARAMETERS
# Templates parameters:
# Number of POI
npoi = 3600
# Number of dimensions in linear subspace
p = 14
# target memory usage (GB), yet it may consumes a bit more.
memory_limit = 80
# datasets location (use download.py to download them)
dataset_dir = "./traces/"
######################### END OF USER PARAMETERS

# Get command line arguemnets
parser = argparse.ArgumentParser()
parser.add_argument("-d", "--dshares", type=int, default=3, help="Number of shares")

parser.add_argument(
    "-n",
    "--ntracesattack",
    type=str,
    default="100",
    help="Number of traces for the attack. Must be integer separated with comma",
)

args = parser.parse_args()
D = args.dshares
ntraces_a = args.ntracesattack.replace(" ", "").split(",")
ntraces_a_all = [int(x) for x in ntraces_a]

# directories for traces
profile_dir = f"{dataset_dir}/sw{D}/random_key/"
attack_dir = [f"{dataset_dir}/sw{D}/fixed_key/key_{x}/" for x in range(5)]

# target source files
sw_src_dir = "./spook_sw/"
# where profiling data will be stored
data_dir = f"./data_{D}/"
# where labels will be stored
label_dir = os.path.join(data_dir, "labels")
if not os.path.exists(label_dir):
    os.makedirs(label_dir)

# where the final models will be stored
models_file = os.path.join(data_dir, f"models_{D}.pkl")
# files with SNR
snr_file = os.path.join(data_dir, f"snr_{D}.pkl")
# file to compute the attack results
attack_summary_file = os.path.join(data_dir, f"attack_summary_{D}.pkl")
# prefix of profiling traces
profile_prefix = os.path.join(profile_dir, f"rkey_sw{D}_10000")

# base variable labels
variables = [
    "a",
    "b",
    "c",
    "d",
    "tmp0",
    "tmp1",
    "tmp2",
    "tmp3",
    "y0",
    "y1",
    "y1r",
    "y2",
    "y3",
]
# number of profiling files
nfiles_profile = 20
# number of traces per profiling file
ntraces_p = 10000


# set traces length for all the D
if D == 3:
    ns = 62500
elif D == 4:
    ns = 83333
elif D == 6:
    ns = 156250
elif D == 8:
    ns = 218750

# Set batch parameters

memory_limit *= 1e9
# remove size of a (int16) trace file
memory_limit -= ns * ntraces_p * 2

memory_per_snr = ns * 8 * 2 * 256
np_snr = int(np.floor(memory_limit / memory_per_snr))
memory_per_lda = (npoi ** 2 + npoi * 256) * 8 + (ntraces_p * npoi) * 8
np_lda = int(np.floor(memory_limit / memory_per_lda))
memory_per_enc_graph = len(variables) * (D + 1) * 256 * 8 * 3
batch_enc = int(np.floor(memory_limit / (memory_per_enc_graph)))
