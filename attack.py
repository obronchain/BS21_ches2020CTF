from env_parse import *
from scalib.attacks import SASCAGraph
from scalib.postprocessing import rank_accuracy
import numpy as np
import glob
import pickle
from tqdm import tqdm
# Encoding graph description
enc_desc = "NC 256\n"
for var in variables:
    for shift in range(4):
        enc_desc += f"""VAR MULTI {var}_{shift}\n"""
        for d in range(D):
            enc_desc += f"""VAR MULTI {var}_{d}_{shift}\n"""

        sums = "^".join([f"{var}_{d}_{shift}" for d in range(D)])
        enc_desc += f"PROPERTY {var}_{shift} = {sums}\n"

# umsk graph description
umsk_desc = "NC 256\n"
id_table = ",".join([f"{x}" for x in np.arange(256)])
umsk_desc += f"TABLE id = [{id_table}]\n"
for shift in range(4):
    for var in variables:
        umsk_desc += f"""VAR MULTI {var}_{shift}\n"""

    umsk_desc += f"""
    VAR SINGLE k_0_{shift}
    VAR SINGLE k_1_{shift}
    VAR SINGLE k_2_{shift}
    VAR SINGLE k_3_{shift}

    VAR MULTI n_0_{shift}
    VAR MULTI n_1_{shift}
    VAR MULTI n_2_{shift}
    VAR MULTI n_3_{shift}

    PROPERTY a_{shift} = k_0_{shift} ^ n_0_{shift}
    PROPERTY b_{shift} = k_1_{shift} ^ n_1_{shift}
    PROPERTY c_{shift} = k_2_{shift} ^ n_2_{shift}
    PROPERTY d_{shift} = k_3_{shift} ^ n_3_{shift}

    PROPERTY tmp0_{shift} = a_{shift} & b_{shift}
    PROPERTY y1_{shift} = tmp0_{shift} ^ c_{shift}
    PROPERTY y1r_{shift} = id[y1_{shift}]

    PROPERTY tmp1_{shift} = d_{shift} & a_{shift}
    PROPERTY y0_{shift} = tmp1_{shift} ^ b_{shift}

    PROPERTY tmp2_{shift} = y1r_{shift} & d_{shift}
    PROPERTY y3_{shift} = tmp2_{shift} ^ a_{shift}

    PROPERTY tmp3_{shift} = y1r_{shift} & y0_{shift}
    PROPERTY y2_{shift} = tmp3_{shift} ^ d_{shift}
    """

models = pickle.load(open(models_file, "rb"))


def run_attack(attack_dir, ntraces_a, models):
    """Run an attack on traces found in a directory.

    Parameters
    ----------
    attack_dir :
        directory containing the traces in *.npz files and the key in a *.txt
        file.
    ntraces_a :
        number of traces for the attack.
    models :
        dictionary containing all the models.

    Returns
    -------
        rmin :
            lower bound for log2 key rank
        r :
            Estimated log2 key rank
        rmax :
            Upper bound for log2 key rank
    """

    # The attack goes in three steps:
    #   1. Retrieve distribution on unshared variables thanks to encoding graphs.
    #   This can be memory consuming and is so done with only batch_enc traces
    #   at the time. The obtained distribution are then added to umsk_graph.
    #   2. Solve the unmasked circuit (umsk_graph) to obtain distribution of the
    #   key bytes.
    #   3. Evaluate the attack with a rank estimation method.

    print("\nRunning Attack on ", attack_dir)
    # Generate umsk_graph
    umsk_graph = SASCAGraph(umsk_desc, n=ntraces_a)

    # Distribution array for all variables in umsk_graph.  They are of shape
    # (ntraces_a,256)
    distributions = {}
    for v in variables:
        for shift in range(4):
            distributions[f"{v}_{shift}"] = np.zeros((ntraces_a, 256))

    # The nonces that will be placed in the umsk_graph.
    n = {}
    for shift in range(4):
        for w in range(4):
            n[f"n_{w}_{shift}"] = np.zeros(ntraces_a, dtype=np.uint32)

    # Traces left to add to umsk_graph
    left = ntraces_a
    # Traces already inserted in umsk_graph
    i = 0

    # get files with the traces
    files_traces = glob.glob(attack_dir + "/*.npz")
    files_traces.sort()

    for files in files_traces:
        dic = np.load(files, allow_pickle=True)
        traces = dic["traces"]
        nonces = dic["nonces"]

        # Go through the traces and nonces in the files with steps of at most
        # batch_enc size.
        for start in range(0, len(traces), batch_enc):
            bs = min(min(batch_enc, len(traces) - start), left)
            print(f"Solve enc graph with {bs} traces. Elapsed {i}/{ntraces_a}")

            # Generate encoding graph
            enc_graph = SASCAGraph(enc_desc, n=bs)

            # predict_proba for all models and insert the result in enc_graph
            for k, m in tqdm(models.items()):
                t = traces[start : start +bs, m["poi"]]
                print("t loaded")
                prs =  m["lda"].predict_proba(t)
                enc_graph.set_init_distribution(
                    k,prs
                )

            # run belief propagation
            enc_graph.run_bp(1)

            # add result in distributions for umsk_graph
            for k, distri in distributions.items():
                distri[i : i + bs, :] = enc_graph.get_distribution(k)

            # add nonces values for umsk_graph
            for shift in range(4):
                for w in range(4):
                    n[f"n_{w}_{shift}"][i : i + bs] = (
                        nonces[start : start + bs, w] >> (shift * 8)
                    ) & 0xFF
            del enc_graph

            # update indexes
            left -= bs
            i += bs
            if left == 0:
                break
        del traces, nonces
        if left == 0:
            break

    print("Solve umsk graph")
    umsk_graph = SASCAGraph(umsk_desc, n=ntraces_a)

    # Set nonces as public variables
    for k, n in n.items():
        umsk_graph.set_public(k, n)

    # Set variables distributions
    for k, distri in distributions.items():
        umsk_graph.set_init_distribution(k, distri)

    # Run belief propagation
    umsk_graph.run_bp(20)

    # Get the key and parse it
    file_key = glob.glob(attack_dir + "/*.txt")[0]

    with open(file_key) as fp:
        full_key = int(fp.readline(), 16)
    secret_key = {}
    for w in range(4):
        for shift in range(4):
            secret_key[f"k_{w}_{shift}"] = ((full_key >> (w * 32))) >> (
                shift * 8
            ) & 0xFF

    guess = []
    rank = []
    ks = []
    distris = []
    # evaluate key rank, key distributions
    for label, k in secret_key.items():
        distri = umsk_graph.get_distribution(label)[0, :]
        ks.append(k)
        guess.append(np.argmax(distri))
        rank.append(256 - np.where(np.argsort(distri) == k)[0])
        distris.append(distri)
    # Run rank estimation with 1-bit precison
    distris = np.array(distris)
    rmin, r, rmax = rank_accuracy(-np.log10(distris), ks, 1.0)

    print("    guess      :", " ".join(["%3x" % (x) for x in guess]))
    print("    best key   :", " ".join(["%3x" % (x) for x in ks]))
    print("    rank       :", " ".join(["%3d" % (x) for x in rank]))
    print("    key rank : %f < %f < %f" % (np.log2(rmin), np.log2(r), np.log2(rmax)))

    del umsk_graph
    return rmin, r, rmax


if __name__ == "__main__":
    attacks = {}
    for attack_dir in attack_dir:
        if not os.path.exists(attack_dir):
            continue
        attacks[attack_dir] = {"n": [], "lr": []}
        for ntraces_a in ntraces_a_all:
            _, lr, _ = run_attack(attack_dir, ntraces_a, models)

            attacks[attack_dir]["n"].append(ntraces_a)
            attacks[attack_dir]["lr"].append(lr)

    pickle.dump(attacks, open(attack_summary_file, "wb"))
