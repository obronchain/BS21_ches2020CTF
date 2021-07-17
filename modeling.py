from env_parse import *
from scalib.modeling import MultiLDA
import scalib
from tqdm import tqdm
import pickle

print(f"Start modeling for {D}-shares")
# load models containing "SNR" field
models = pickle.load(open(snr_file, "rb"))

snr_at_poi = {}
# Compute pois
for v,m in models.items():
    # to avoid NaN if scope overshoot
    np.nan_to_num(m["SNR"])
    # take the npoi with largest SNR values
    poi = np.argsort(m["SNR"])[-npoi:].astype(np.uint32)
    poi = np.sort(poi)
    snr_at_poi[v] = {"poi":poi,"snr":m["SNR"][poi]}

pickle.dump(snr_at_poi, open(snr_file_at_poi, "wb"))

# File for profiling
files_traces = [f"{profile_prefix}_{x}.npz" for x in range(nfiles_profile)]
files_labels = [
    os.path.join(label_dir, f"label_{D}_{x}.pkl") for x in range(nfiles_profile)
]

labels_model = list(snr_at_poi)
split = [labels_model[i : i + np_lda] for i in range(0, len(labels_model), np_lda)]

res = {"mlda": [], "labels": []}
for b, labels_batch in enumerate(split):

    # Number of variables in this round
    np_it = len(labels_batch)

    # Init the MultiLDA for the labels to profile
    pois = [snr_at_poi[v]["poi"] for v in labels_batch]
    mlda = MultiLDA(ncs=[256] * np_it, ps=[p] * np_it, pois=pois, gemm_mode=1)

    files_labels = tqdm(files_labels, desc="Load batch %d/%d" % (b, len(split)))
    for (traces, labels, index) in zip(
        files_traces, files_labels, range(0, ntraces_p * nfiles_profile, ntraces_p)
    ):

        # load traces and labels
        traces = np.load(traces, allow_pickle=True)["traces"]
        labels = pickle.load(open(labels, "rb"))

        labels = np.array([labels[v] for v in labels_batch], dtype=np.uint16).T
        mlda.fit_u(traces, labels)
        del traces, labels

    mlda.solve(done=True)
    res["mlda"].append(mlda)
    res["labels"].append(labels_batch)

pickle.dump(res, open(models_file, "wb"))
