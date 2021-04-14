BS20 CHES2020 CTF Attack
========================

This repository contains the attack by `Bronchain and Standaert` against the
CHES2020 CTF software targets. It leverages `Soft Analytical Side-Channel
Attacks` and is using its implementation in `SCALE`. It allows to break all
the software implementations protected with 3,4,6 and 8 shares. 

This is an artifact of the research paper `Breaking Masked Implementations with
Many Shares on 32-bit Software Platforms or When the Security Order Does Not
Matter` published in TCHES 2021, Issue 3. 

Overview
========
This project goes in three steps:

1. **Profile**: Which consists in first derived the values of each intermediate
   variable, computing `SNR` and build a template for each of them.
2. **Attack**: Which consists in recovering the secret keys by leveraging `Soft
   Analytical Side-Channel Attacks (SASCA)`.
3. **Report**: Which consists in plotting the profiling and attack results.

Since the output of profiling and attacks are made available online, each step
can be done independently (profiling is the more costly).

System requirement
==================
In order to run the project, at least a Linux systems is required with `Python
>=3.6`, `curl` and `gcc` installed (default on most of modern distributions).
Since some steps of the computation can be expensive, all the outputted results
are made available online and can be downloaded with `download.py`. 

To run the full package (profiling and attack), we recommend to have at least
32-GB of RAM. The user can specify the memory he wants to dedicate to it by
changing `memory_limit` in `env_parse.py`. More RAM is welcome since it allows
a better usage of parallelism. The artifact has been tested with 90 GB of RAM
and 48 threads.  

To run profiling and/or attacks, traces must be downloaded from the CHES2020
CTF website and stored on the disk (with `download.py`). For the simplest
target (3-shares), it represents about 36 GB of data. We note that the user is
not forced to download all the dataset. See the next section for more details. 

Parameters
==========
A few parameters can be tuned by the user in `env_parse.py`. We list and describe them below:

- `memory_limit`: The target RAM (in GB) usage. Because Python has garbage collection, this limit can be (slightly) exceeded.  

- `dataset_dir`: Is the directory containing the datasets. The downloaded traces will be placed there. If you already have the traces available on your system, make `dataset_dir` pointing to their location.

- `npoi`: The number of dimensions in the traces taken to build the templates.

- `p`: The number of dimensions in the considered linear subspace.

Install
=======
All the Python dependencies are available on `PyPi` and can be installed with.

.. code-block:: 

   pip install scale matplotlib

Both `curl` and `gcc` must be installed on the system. This can be done with
the distribution package manager (e.g., apt-get, pacman, etc ...).

Download Datasets
=================
The script `download.py` is an interactive scripts to select what dataset must
be downloaded.  The user can select to download profile and/or attack dataset
for each of the implementations.  The following example downloads both datasets
for the 3-shares implementation. 

.. code-block::

    bash: python3 download.py

    Confirm what you want to download. You can edit to top of this file.

    About to download 26.00-GB for 3-shares profiling dataset. Continue ? [y/n]: y
    About to download 32.00-GB for 4-shares profiling dataset. Continue ? [y/n]: n
    Skipping
    About to download 62.00-GB for 6-shares profiling dataset. Continue ? [y/n]: n
    Skipping
    About to download 82.00-GB for 8-shares profiling dataset. Continue ? [y/n]: n
    Skipping


    About to download 6.50-GB for 3-shares attack datasets. Continue ? [y/n]: y
    About to download 8.00-GB for 4-shares attack datasets. Continue ? [y/n]: n
    Skipping
    About to download 62.00-GB for 6-shares attack datasets. Continue ? [y/n]: n
    Skipping
    About to download 164.00-GB for 8-shares attack datasets. Continue ? [y/n]: n
    Skipping

    ....

**note**: The script does not download entire attack datasets by only enough
traces to recover the full key.

	
Profiling
=========

**note**: All the commands for profiling and attacks are in `all.sh`.

The profiling is done by running sequentially the three following scripts where
`<D>` is the number of shares in the implementation to profile.

.. code-block::
   
   python3 gen_labels.py -d <D> 
   python3 compute_snr.py -d <D> 
   python3 modeling.py -d <D>

`gen_labels.py` derives the values for each of the intermediate variables
(share) by using the implementation sources (with additional MACRO).
`compute_snr.py` computes the SNR for each of the variables. `modeling.py`
builds the templates.

In order display the results of profiling, the user can start the interactive
script `report_profiling.py`. The user will be asked the variables and the byte index
(0,1,2 or 3) to display.

.. code-block::
   
   python3 report_profiling.py -d <D>
 

**warning**: Profiling is the most expensive steps. See Section 4. of the paper
for additional details about complexities. 

Attack
======
The attack can be executed by running the scripts

.. code-block::

   python3 attack.py -d <D> -n <n1,n2,n3,..>

where `ni` is the number of points to consider for the attack. The attacks are
performed on each of the 5 datasets.

The results can be reported with the script `report_attack.py`. This report on
the x-axis the number of traces in the attack and on the y-axis the full key
rank (log2-scale). The crosses are for individual attack dataset (out of 5) and
the red curve is the median.

.. code-block::
   
   python3 report_attack.py -d <D>

Contact
=======
For additional information, issues and suggestions, please contact Olivier
Bronchain at `olivier.bronchain@uclouvain.be
<olivier.bronchain@uclouvain.be>`_. 

License
=======
All the files in this project (expected for `spook_sw/`) are distributed under AGPLv3. Please see `<COPYING>`_ for additional information.  
