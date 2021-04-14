from env_parse import *
from scaffe.metrics import SNR
from tqdm import tqdm
import pickle

print(f"Start SNR estimation for {D}-shares")

# Create models for every variables
models = {}
for shift in range(4):
    for d in range(D):
        for var in variables:
            models[f"{var}_{d}_{shift}"] = {}

# File for profiling
files_traces = [f"{profile_prefix}_{x}.npz" for x in range(nfiles_profile)]
files_labels = [os.path.join(label_dir,f"label_{D}_{x}.pkl") for x in
                        range(nfiles_profile)]

# Go over all the profiling traces and update the SNR Since computing for all
# the variables at once may overlead the RAM, We compute SNR for matches of size
# np_snr.
labels_model = list(models)
split = [labels_model[i:i + np_snr] for i in range(0, len(labels_model), np_snr)]

for b,labels_batch in enumerate(split):

    # Number of variables in this round
    np_it = len(labels_batch)

    # Go through all files and update SNR in one pass for labels_model
    files_labels = tqdm(files_labels,desc="Batch %d/%d"%(b,len(split)))
    for it,(traces,labels) in  enumerate(zip(files_traces,files_labels)):

        # load traces and labels
        traces = np.load(traces,allow_pickle=True)["traces"]
        labels = pickle.load(open(labels,"rb"))

        # Init labels and data array on the first iteration
        if it == 0:
            snr = SNR(np=np_it,nc=256,ns=ns)
            data = np.zeros((ntraces_p,np_it),dtype=np.uint16)
        
        # fill data array
        for i,v in enumerate(labels_batch):
            data[:,i] = labels[v]

        # update SNR estimate
        snr.fit_u(traces,data)
        del traces

    snr_val = snr.get_snr()
    for i,v in enumerate(labels_batch):
        models[v]["SNR"] = snr_val[i,:].copy()
    
    del snr 

pickle.dump(models,open(snr_file,'wb'))
