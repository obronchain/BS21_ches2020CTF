PYTHON=python3

# 3 shares
$PYTHON gen_labels.py -d 3
$PYTHON compute_snr.py -d 3
$PYTHON modeling.py -d 3
$PYTHON attack.py -d 3 -n 1,2,4,6,11,18,29,48,78,127,206,335,545,885,1438,2335,3792,6158,10000

# 4 shares
$PYTHON gen_labels.py -d 4
$PYTHON compute_snr.py -d 4
$PYTHON modeling.py -d 4
$PYTHON attack.py -d 4 -n 1,2,4,6,11,18,29,48,78,127,206,335,545,885,1438,2335,3792,6158,10000

# 6 shares
$PYTHON gen_labels.py -d 6
$PYTHON compute_snr.py -d 6
$PYTHON modeling.py -d 6
$PYTHON attack.py -d 6 -n 1,3,5,9,16,28,49,86,151,264,461,806,1408,2460,4297,7506,13110,22900,40000

# 8 shares
$PYTHON gen_labels.py -d 8
$PYTHON compute_snr.py -d 8
$PYTHON modeling.py -d 8
$PYTHON attack.py -d 8 -n 1,3,5,10,19,35,64,116,210,380,689,1249,2263,4100,7427,13456,24376,44160,80000
