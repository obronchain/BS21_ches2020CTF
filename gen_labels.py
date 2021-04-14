import os
from env_parse import *
from subprocess import Popen, PIPE
from tqdm import tqdm
import pickle
if __name__ == "__main__":
    cmd = f"make -C {sw_src_dir} D={D}"

    print(f"Compiling implementation sources in {sw_src_dir}\n")
    os.system(cmd)

    print(f"\nFor each file, generate the labels")
    for fid,filename in enumerate(tqdm([profile_prefix+f"_{x}.npz" for x in
                            range(nfiles_profile)])):

        labels = {}
        for shift in range(4):
            for d in range(D):
                for var in variables:
                    labels[f"{var}_{d}_{shift}"] = np.zeros(ntraces_p,dtype=np.uint8)

        # Get inputs to the implementation
        dic = np.load(filename,allow_pickle=True)
        seeds = dic["seeds"]
        nonces = dic["nonces"]
        keys = dic["msk_keys"]
        tk = np.zeros((ntraces_p,4),dtype=np.uint32)
        datas = np.concatenate((seeds,nonces,tk,keys),axis=1)

        # Start the implementation
        p = Popen(f'./{sw_src_dir}./implem', stdin=PIPE,stdout=PIPE)
        outs,err = p.communicate(datas.tobytes())
        lines = outs.decode()
        for i,lines in enumerate(lines.split("=")[1:]):
            lines = lines.split("\n")
            for label,value in zip(lines[::2],lines[1::2]):
                value = int(value,16)
                for shift in range(4):
                    labels[label+f"_{shift}"][i] = (value >> (8*shift))&0xff
        pickle.dump(labels,open(os.path.join(label_dir,f"label_{D}_{fid}.pkl"),"wb"))
    
    cmd = f"make -C {sw_src_dir} clean"
    os.system(cmd)
