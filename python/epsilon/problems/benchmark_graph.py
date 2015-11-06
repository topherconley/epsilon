from collections import defaultdict
import sys

from matplotlib import pyplot as plt
import matplotlib

# FONT = {
#     "family": "normal",
#     "weight": "normal",
#     "size": 14,
# }
# matplotlib.rc("font", **FONT)

LABELS = {
    'epsilon': 'Epsilon',
    'scs': 'CVXPY+SCS',
    'ecos': 'CVXPY+ECOS',
}

if __name__ == "__main__":
    data = defaultdict(list)
    for line in sys.stdin:
        label, n, t = line.split()
        data[label].append((n, t))

    for label, n_t in data.iteritems():
        plt.loglog([n for n, t in n_t],
                   [t for n, t in n_t],
                   label=LABELS[label], linewidth=2)

    plt.autoscale(tight=True)
    plt.legend(loc="upper left")
    plt.ylabel("Running time (seconds)")
    plt.xlabel("Number of variables")
    plt.grid(b=True, which="major", linestyle="-")
    plt.grid(b=True, which="minor", linestyle="--")
    plt.savefig(sys.stdout, format="pdf")
