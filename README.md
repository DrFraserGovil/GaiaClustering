# GaiaClustering

This is a C++ code used to calculate the probability, P(m <=5|k), that the number of clusters formed from a full time series of N points is  5 or fewer, when only a subset k <=N points are available. Here a set of datapoints are considered to be in `a cluster' if they are separated by less than 4 days. 


## Compilation

The code can be compiled using the makefile included, or manually using the command:

```g++ ClusteringDistribution.cpp -o clusters -pthread```

This code has been tested and has been found to be safe when run with `Ofast`, so if performance is critical, we recommend using:

```g++ ClusteringDistribution.cpp -o clusters -pthread -Ofast -march=native```

## Command Line Arguments

The code accepts up to 3 command line arguments:

``` ./clusters ARG1  ARG2  ARG3 ```

ARG1 is expected to be a string containing the relative path to the input file. Similarly, ARG2 changes the target location for the output file. ARG3 is an integer value for the number of cores on which to execute the parallel part of the code (recall that since one core will remain the `parent' core, this number should be at most one fewer than the total number of cores available). 

Not all arguments have to be provided, and if they are missed, the code will use the default values:

* ARG1 = `gaia_t_maps_128.csv`
* ARG2 = `ClusteringDistribution.dat`
* ARG3 = `3`

An example run would be:

``` ./clusters gaia_t_maps_1024.csv largeOutput.dat 10 ```

This would run the code on the much larger `1024` file using 10 child cores (11 in total) and save the output to `largeOutput.dat`, and then the sorted output to `sorted_largeOutput.dat`. 

## Input & Output Files

The expected input is a comma-delmited file with the time series forming horizontal rows. The code ignores the first two entries on each column (assuming them to be metadata), though this can be modified by changing the value of `dataOffset` in the `processLine` function. 

For each line in the input file, the code generates a corresponding comma-delimited line in the output file, with the first column being the (padded) line ID, and then every subsequent column being P(m<=5|k,ID) for increasing `k`, starting from `k=0`. The number of entries per line is not constant. 

Due to the parallel nature of the code, though the file read-in is sequential, the output file `output.dat` is often mixed up. The code calls the linux `sort` command after completing the calculations, and saves this result in a second output file, `sorted_output.dat`. 
