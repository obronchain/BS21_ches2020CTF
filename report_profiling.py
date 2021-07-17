from env_parse import *
import pickle
import matplotlib.pyplot as plt

snr_at_poi = pickle.load(open(snr_file_at_poi, "rb"))
snrs = None
if os.path.exists(snr_file):
    snrs = pickle.load(open(snr_file, "rb"))

print(f"Welcome to report SNR for {D} shares")
print("You will be asked to select variables for which you want to plot SNR\n")
print("Corresponds to the byte position in the 32-bit register")
print("On plots, the colors are for different shares.")

print("\nVariables are: ", ", ".join(variables))

while True:
    # get variables to plot
    requested_var = input(
        """\nPlease select a variable to report. 
Multiple variables must be separated with a comma: """
    )
    requested_var = requested_var.replace(" ", "").split(",")
    if not all([x in variables for x in requested_var]):
        print("\nUser Error: One variable is not available")
        print("Variables are: ", ", ".join(variables),"\n")
        continue

    # get the shift to plot
    requested_shift = input("Select a shift in [0,1,2,3]: ").replace(" ", "")
    if requested_shift not in ["0", "1", "2", "3"]:
        print("\n User Error: Uncorrect shift\n")
        continue
    shift = int(requested_shift, 10)

    plt.figure()
    ax = None
    for i, var in enumerate(requested_var):
        ax = plt.subplot(len(requested_var), 1, i + 1, sharex=ax)
        for d in range(D):
            v = f"{var}_{d}_{shift}"
            poi = snr_at_poi[v]["poi"]
            if not snrs is None:
                ax.plot(snrs[v]["SNR"], alpha=0.7)
            ax.scatter(poi, snr_at_poi[v]["snr"], alpha=0.7, marker="x")
            ax.set_ylabel(f"{var} shift {shift}")
        plt.grid()

    plt.show(block=False)
