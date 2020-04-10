

# Efficient Inference of Optimal Decision Trees

Paper link: http://florent.avellaneda.free.fr/dl/AAAI20.pdf

The source code is designed to be compiled and executed on GNU/Linux.

## Dependencies

- g++
- cmake
- minisat : http://minisat.se/
- xdot : https://github.com/jrfonseca/xdot.py
- CLI11 : https://github.com/CLIUtils/CLI11

### Example of installing dependencies for Ubuntu

The following instructions have been tested on Ubuntu 19.04

```bash
sudo apt install g++ cmake minisat2 xdot libzip-dev libboost-dev
```

## Build

```bash
cmake .
make
```

## Running

Infer a decision tree with the algorithm *DT_depth* for the dataset "mouse":

```bash
./InferDT -d data/mouse.csv infer
```

Infer a decision tree with the algorithm *DT_size* for the dataset "car":

```bash
./InferDT data/car.csv infer
```

Run a 10-cross-validation on the dataset "mouse" with the algorithm *DT_size*:

```bash
./InferDT data/mouse.csv bench
```

Infer a decision tree with the algorithm *DT_size* for the training set balance-scale.csv.train1 and evaluate the model with the testing set balance-scale.csv.test1:

```bash
./InferDT data/balance-scale.csv.train1.csv infer -t data/balance-scale.csv.test1.csv
```

Print a help message:

```bash
./InferDT --help
```

## Benchmarks

We ran experiments on Ubuntu with Intel Core CPU i7-2600K @ 3.40 GHz.

### Verwer and Zhang Datasets
The datasets we used are extracted from the paper of Verwer and Zhang and are available at
(https://github.com/SiccoVerwer/binoct).

| Dataset       |  S   |  B   | Time  DT_depth | Accuracy DT_depth |  k   | n for DT_size | Time  DT_size | Accuracy  DT_size | Accuracy BinOCT* |
| ------------- | :--: | :--: | :------------: | :---------------: | :--: | :-----------: | :-----------: | :---------------: | ---------------- |
| iris          | 150  | 114  |     18 ms      |      92.9 %       |  3   |     10.6      |     30 ms     |      93.2 %       | **98.4 %**       |
| monks1        | 124  |  17  |     24 ms      |      90.3 %       | 4.4  |      17       |     80 ms     |    **95.5 %**     | 87.1 %           |
| monks2        | 169  |  17  |     190 ms     |      70.2 %       | 5.8  |     47.8      |    9.1 sec    |    **74.0 %**     | 63.3 %           |
| monks3        |  122  |  17  |     30 ms      |      78.1 %       | 4.8  |     23.4      |    210 ms     |      82.6 %       | **93.5 %**       |
| wine          | 178  | 1276 |     600 ms     |      89.3 %       |  3   |      7.8      |    1.2 sec    |    **92.0 %**     | 88.9 %           |
| balance-scale | 625  |  20  |     50 sec     |    **93.0 %**     |  8   |      268      |    183 sec    |      92.6 %       | 77.5 %           |

**S**: Number of examples in the dataset

**B**: Number of Boolean features in the dataset

**Time  DT_depth**: Time used by our algorithm with parameter "-d"

**Accuracy DT_depth**: Accuracy of our algorithm with parameter "-d"

**k**: Depth of inferred decision trees

**n**: Number of nodes in inferred decision trees

**Time  DT_size**: Time used by our algorithm without parameter "-d"

**Accuracy DT_size**: Accuracy of our algorithm without parameter "-d"

**Accuracy BinOCT**: Accuracy of algorithm from https://github.com/SiccoVerwer/binoct

### Mouse

We used dataset [Mouse](https://raw.githubusercontent.com/FlorentAvellaneda/InferDT/master/data/mouse.csv) that the authors Bessiere, Hebrard and O'Sullivan shared with us. Each entry in rows DT\_size and DT\_depth corresponds to the average over 100 runs. The first columns correspond to the name of each algorithm used. The next three columns correspond to inferring a decision tree from the whole dataset. The last column corresponds the 10-fold cross-validations.

| Algorithm                                                    |   Time   |  k   |  n   | Accuracy |
| ------------------------------------------------------------ | :------: | :--: | :--: | :------: |
| DT2 from paper [Minimising Decision Tree Size as Combinatorial Optimisation](http://homepages.laas.fr/ehebrard/papers/cp2009b.pdf) | 577 sec  |  4   |  15  |  83.8 %  |
| DT1 from paper [Learning Optimal Decision Trees with SAT](https://www.ijcai.org/proceedings/2018/0189.pdf) | 12.9 sec |  4   |  15  |  83.8 %  |
| DT_size: our algorithm without parameter "-d"                |  70 ms   |  4   |  15  |  83.5 %  |
| DT_depth: our algorithm with parameter "-d"               |  20 ms   |  4   |  31  |  85.8 %  |

### Other

In this section we perform **x** 10-cross-validations made randomly and record the average.


| Dataset                                                      |  S   |  B   | Time  DT_depth | Accuracy DT_depth |  k   | n for DT_size | Time  DT_size | Accuracy  DT_size | x    |
| ------------------------------------------------------------ | :--: | :--: | :------------: | :---------------: | :--: | :-----------: | :-----------: | :---------------: | ---- |
| [zoo](http://archive.ics.uci.edu/ml/datasets/Zoo)            | 101  | 136  |     47 ms      |      91.7 %       |  4   |      20       |    200 ms     |       91 %        | 200  |
| [BodyMassIndex](https://www.kaggle.com/yersever/500-person-gender-height-weight-bodymassindex) | 500  | 172  |     53 sec     |       85 %        | 6.6  | 109              |  6.4 h             |  85.4 %                 | 1    |
| [lungCancerDataset](https://www.kaggle.com/yusufdede/lung-cancer-dataset) |  59  |  72  |     3.8 ms     |      89.6 %       | 2.6  |       7       |    7.5 ms     |      90.6 %       | 200  |
| [iris](https://raw.githubusercontent.com/FlorentAvellaneda/InferDT/master/data/iris.csv) |  113  |  114  |     30 ms     |      93.6 %       | 3.6  |       14       |    120 ms     |      94.0 %       | 200  |
| [monks1](https://raw.githubusercontent.com/FlorentAvellaneda/InferDT/master/data/monks1.csv) |  124  |  17  |     30 ms     |      97.0 %       | 4.9  |       15       |    140 ms     |      99.5 %       | 200  |
| [monks2](https://raw.githubusercontent.com/FlorentAvellaneda/InferDT/master/data/monks2.csv) |  169  |  17  |     330 ms     |      88.0 %       | 6  |       64       |    9.3 sec     |      88.7 %       | 100  |
| [monks3](https://raw.githubusercontent.com/FlorentAvellaneda/InferDT/master/data/monks3.csv) |  92  |  17  |     125 ms     |      79.6 %       | 5.8  |       35.5       |    2.5 sec     |      81.5 %       | 800  |
| [balance-scale](https://raw.githubusercontent.com/FlorentAvellaneda/InferDT/master/data/balance-scale.csv) |  625  |  20  |      200 sec     |      71.8 %       | 9  |       276       |    11 h     |      73.6 %       | 1  |
| [wine](https://raw.githubusercontent.com/FlorentAvellaneda/InferDT/master/data/wine.csv) |  178  | 1276  |      4.5 sec     |      91.5 %       | 3  |        12.2      |    14.5 sec     |      91.2 %       | 200  |

**S**: Number of examples in the dataset

**B**: Number of Boolean features in the dataset

**k**: Average depth of inferred decision trees

**n**: Average number of nodes in inferred decision trees

**x**: Number of 10-cross-validation performed







