import requests
import os
from env_parse import *

#### User parameters
# profiling sets (number of shares)
profiling_datasets = ["3",
                        "4",
                        "6",
                        "8"]
# attack sets (number of shares and attack datasets)
attack_datasets = {"3":[0,1,2,3,4],
                    "4":[0,1,2,3,4],
                    "6":[0,1,2,3,4],
                    "8":[0,1,2,3,4]}
### End of user parameters

URL_BASE="https://anonymous@enigma.elen.ucl.ac.be/webdav/ctf-spook/ctf_traces1"
sizes = {"3":[1.3,1],
            "4":[1.6,1],
            "6":[3.1,4],
            "8":[4.1,8]}

tsize = 0
files = []
def download_file(fname):
    output = f"{dataset_dir}/{fname}"
    if os.path.exists(output):
        print(f"file {output} exists. Skipping")
    else:
        dirname = os.path.dirname(output)
        if not os.path.exists(dirname):
            os.makedirs(dirname)
        print("--------------")
        print(f"Download {fname}")
        url = f'{URL_BASE}/{fname}'
        os.system(f"curl {url} --output {output}")


def gen_attack_dataset():
    global tsize
    for d,a in attack_datasets.items():
        size = sizes[d][1]*sizes[d][0]
        size = size*len(a)
        ret = input(f"About to download {size:.2f}-GB for {d}-shares attack datasets. "
                +"Continue ? [y/n]: ")
        if ret != "y":
            print("Skipping")
            continue 
        
        tsize += size
        for a in a:
            # directory on the server side
            dir_server = f"sw{d}/fixed_key/key_{a}"
            files.append(f"{dir_server}/secret_sw{d}_K{a}.txt")
            for i in range(sizes[d][1]):
                fname = f"fkey_sw{d}_K{a}_10000_{i}.npz"
                files.append(f"{dir_server}/{fname}")

def gen_profile_dataset():
    global tsize

    for d in profiling_datasets:
        size = sizes[d][0]*nfiles_profile
        ret = input(f"About to download {size:.2f}-GB for {d}-shares profiling dataset. "
                +"Continue ? [y/n]: ")
        if ret != "y":
            print("Skipping")
            continue

        tsize += size
        # directory on the server side
        dir_server = f"sw{d}/random_key"
        for i in range(nfiles_profile):
            fname = f"rkey_sw{d}_10000_{i}.npz"
            files.append(f"{dir_server}/{fname}")

if __name__ == "__main__":
    print("Confirm what you want to download. You can edit to top of this file.\n")
    gen_profile_dataset()
    print("\n")
    gen_attack_dataset()

    print("\n")
    ret = input(f"Start download a total of {tsize:.2f}-GB. Continue ? [y/n]: ")
    if ret != "y":
        exit()
    for f in files:
        download_file(f)

