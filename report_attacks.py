from env_parse import *
import numpy as np
import pickle
import matplotlib.pyplot as plt

attacks = pickle.load(open(attack_summary_file, "rb"))

r = []
plt.figure()

for it, (k, attack) in enumerate(attacks.items()):
    r.append(attack["lr"])
    plt.scatter(attack["n"], np.array(attack["lr"]), alpha=0.7, marker="x", color="b")

r = np.array(r)
plt.loglog(attack["n"], np.median(r, axis=0), basey=2, color="r", label="Median")
plt.grid(True, which="both", ls="--")
plt.xlabel("Number of traces")
plt.ylabel("key rank")
plt.title(f"Attack summary for {D}-shares Clyde")
plt.axhline(2.0 ** 128, ls="--", color="r")
plt.axhline(2 ** 32, ls="--", color="g")
plt.axhline(2 ** 0, ls="--", color="b")
plt.yticks(2.0 ** np.arange(0, 129, 8))
plt.show()
