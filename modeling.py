from env_parse import *
from scalib.modeling import LDAClassifier
import scalib
from tqdm import tqdm
import pickle

print(f"Start modeling for {D}-shares")
# load models containing "SNR" field
models = pickle.load(open(snr_file,"rb"))

# Compute pois
for m in models.values():
    # to avoid NaN if scope overshoot
    np.nan_to_num(m["SNR"])
    m["poi"] = np.argsort(m["SNR"])[-npoi:].astype(np.uint32)
    m["poi"] = np.sort(m["poi"])
    m["SNR_at_poi"] = m["SNR"][m["poi"]]
    m.pop("SNR")

# File for profiling
files_traces = [f"{profile_prefix}_{x}.npz" for x in range(nfiles_profile)]
files_labels = [os.path.join(label_dir,f"label_{D}_{x}.pkl") for x in
                        range(nfiles_profile)]

labels_model = list(models)
split = [labels_model[i:i + np_lda] for i in range(0, len(labels_model), np_lda)]

for b,labels_batch in enumerate(split):

    # Number of variables in this round
    np_it = len(labels_batch)

    files_labels = tqdm(files_labels,desc="Load batch %d/%d"%(b,len(split)))
    for (traces,labels,index) in zip(files_traces,
                                    files_labels,
                                    range(0,ntraces_p*nfiles_profile,ntraces_p)):
        
        # load traces and labels
        traces = np.load(traces,allow_pickle=True)["traces"]
        labels = pickle.load(open(labels,"rb"))

        # Init labels and data array on the first iteration
        if index == 0:
            for v in labels_batch:
                m = models[v]
                m["l"] = np.zeros((ntraces_p*nfiles_profile,len(m["poi"])),
                                        dtype=np.int16)
                m["x"] = np.zeros((ntraces_p*nfiles_profile),
                                        dtype=np.uint16)
        
        # fill data array
        for v in labels_batch:
            m = models[v]
            
            # Equivalent of: m["l"][index:index+ntraces_p,:] = traces[:,m["poi"]]
            scalib._scalib_ext.partial_cp(traces,m["poi"],m["l"][index:index+ntraces_p])
            
            m["x"][index:index+ntraces_p] = labels[v]
        del traces

    for v in tqdm(labels_batch,
            desc="Profile batch %d/%d"%(b,len(split))):

        m = models[v]
        l = m.pop("l")
        x = m.pop("x")
        m["lda"] = LDAClassifier(nc=256,ns=len(m["poi"]),p=p)
        m["lda"].fit(l,x)
        del l,x

pickle.dump(models,open(models_file,'wb'))
