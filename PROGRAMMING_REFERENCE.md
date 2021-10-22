Table of Contents:
- [1. C/C++](#1-cc)
  - [1.1. API Reference](#11-api-reference)
  - [1.2. Supported Features by Compiler Version](#12-supported-features-by-compiler-version)
  - [1.3. C++ Libraries](#13-c-libraries)
    - [1.3.1. GNU libstdc++](#131-gnu-libstdc)
    - [1.3.2. List of Libraries](#132-list-of-libraries)
    - [1.3.3. GUI](#133-gui)
    - [1.3.4. Intel TBB](#134-intel-tbb)
  - [1.4. Replacing STL allocators:](#14-replacing-stl-allocators)
  - [1.5. Cheat sheets](#15-cheat-sheets)
- [2. Algorithms](#2-algorithms)
  - [2.1. Reference](#21-reference)
  - [2.2. Interviewing and Competitive Programming](#22-interviewing-and-competitive-programming)
- [3. Design Patterns and OO](#3-design-patterns-and-oo)
  - [3.1. UML syntax](#31-uml-syntax)
- [4. Python](#4-python)
  - [4.1. Tutorials](#41-tutorials)
  - [4.2. Cheat Sheets](#42-cheat-sheets)
  - [4.3. profiling](#43-profiling)
  - [4.4. Source Code:](#44-source-code)
- [5. Debugging,Analysis and Performance Tuning](#5-debugginganalysis-and-performance-tuning)
  - [5.1. Automated](#51-automated)
  - [5.2. GDB](#52-gdb)
  - [5.3. Profiling](#53-profiling)
- [6. OS specific](#6-os-specific)
  - [6.1. Linux](#61-linux)
  - [6.2. AIX](#62-aix)
    - [6.2.1. system administration](#621-system-administration)
      - [6.2.1.1. processors and configuration](#6211-processors-and-configuration)
  - [6.3. Docker](#63-docker)
    - [6.3.1. Basic Commands](#631-basic-commands)
- [7. Unix tips](#7-unix-tips)
  - [7.1. Why is computer slow?](#71-why-is-computer-slow)
  - [7.2. Save terminal session with tmux](#72-save-terminal-session-with-tmux)
- [8. Books](#8-books)
- [9. Statistical Analysis and Machine Learning](#9-statistical-analysis-and-machine-learning)
  - [9.1. Fundamentals of ML](#91-fundamentals-of-ml)
  - [9.2. Statistical measures in python:](#92-statistical-measures-in-python)
    - [9.2.1. Plotting:](#921-plotting)
    - [9.2.2. Fitting:](#922-fitting)
    - [9.2.3. Sampling:](#923-sampling)
    - [9.2.4. Learning](#924-learning)


# 1. C/C++
## 1.1. API Reference
 * CPP: :boom: https://en.cppreference.com/w/cpp
 * CPP: https://www.cplusplus.com/reference/unordered_map/unordered_map/
 * CPP How to Program: https://cs.fit.edu/~mmahoney/cse2050/how2cpp.html
 * C standard library: https://www.cplusplus.com/reference/clibrary/
## 1.2. Supported Features by Compiler Version
 * https://en.cppreference.com/w/cpp/compiler_support
## 1.3. C++ Libraries
### 1.3.1. GNU libstdc++
* Docs: https://gcc.gnu.org/onlinedocs/libstdc++/manual/index.html
* Library test suite for parallel sort: https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/testsuite/25_algorithms/pstl/alg_sorting/sort.cc;h=e0c55df404f82dc360cea77ce411511bc2b4e619;hb=HEAD
### 1.3.2. List of Libraries
https://en.cppreference.com/w/cpp/links/libs
### 1.3.3. GUI
* SFML https://www.sfml-dev.org/index.php
* TGUI https://tgui.eu/
### 1.3.4. Intel TBB
Forms the basis of parallel algorithms in latest C++ versions

https://software.intel.com/content/www/us/en/develop/documentation/tbb-documentation/top.html

Introduction: https://www.youtube.com/watch?v=9Otq_fcUnPE

## 1.4. Replacing STL allocators:
* https://gcc.gnu.org/onlinedocs/libstdc++/manual/memory.html
* https://johnysswlab.com/the-price-of-dynamic-memory-allocation/
* 
## 1.5. Cheat sheets
 * Syntax cheat sheet: https://github.com/gibsjose/cpp-cheat-sheet/blob/master/C%2B%2B%20Syntax.md
 * Data Structures and Algorithms cheat sheet: https://github.com/gibsjose/cpp-cheat-sheet/blob/master/Data%20Structures%20and%20Algorithms.md
 * C++ STL quick reference: https://overapi.com/static/cs/STL%20Quick%20Reference%201.29.pdf

# 2. Algorithms
## 2.1. Reference
https://www.geeksforgeeks.org/
## 2.2. Interviewing and Competitive Programming
* :boom: https://www.hackerrank.com/
* https://www.hackerrank.com/philip_miloslav1
# 3. Design Patterns and OO
* SOLID:  https://scotch.io/bar-talk/s-o-l-i-d-the-first-five-principles-of-object-oriented-design
* Catalog: https://en.wikipedia.org/wiki/Software_design_pattern
* Cloud Patterns: https://docs.microsoft.com/en-us/azure/architecture/patterns/
* UML: https://www.guru99.com/uml-cheatsheet-reference-guide.html
## 3.1. UML syntax
<<interface>>  +public  -private    variable: type

Inheritance: from class: solid line black arrow, from abstract class: solid line white arrow, from interface: dashed line white arrow

Aggregation: white diamond

Composition: black diamond

# 4. Python
## 4.1. Tutorials
* :boom: Misc: https://realpython.com/
* python source: https://realpython.com/cpython-source-code-guide/
## 4.2. Cheat Sheets
* https://gto76.github.io/python-cheatsheet/ 
## 4.3. profiling
python -m cProfile -s time circuit.py
## 4.4. Source Code:
# 5. Debugging,Analysis and Performance Tuning
## 5.1. Automated
* valgrind
* cachegrind (find cache thrashing)
## 5.2. GDB
* SIGSEGV: http://unknownroad.com/rtfm/gdbtut/gdbsegfault.html
## 5.3. Profiling

# 6. OS specific
## 6.1. Linux
* Search kernel source: https://elixir.bootlin.com/linux/v5.14.10/source
* Search kernel Docs: https://www.kernel.org/doc/html/latest/search.html
### perf
* sudo apt-get update
* sudo apt-get dist-upgrade
* sudo apt-get install --reinstall linux-tools-common linux-tools-generic linux-tools-`uname -r`
* sudo perf top
## 6.2. AIX
### 6.2.1. system administration
```sudo lssecattr -c /usr/sbin/lsattr
1420-012 "/usr/sbin/lsattr" does not exist in the privileged command database.
sudo lssecattr -c /usr/pmapi/tools/pmcycles
/usr/pmapi/tools/pmcycles accessauths=aix.system.pmustat.global innateprivs=PV_PMU_CONFIG,PV_PMU_SYSTEM secflags=FSF_EPS
```

#### 6.2.1.1. processors and configuration
* lscfg -lproc\*
* lparstat -i | grep CPU
* also lparstat without args (shows smt4 sometimes)
* bindprocessor -q
* lsattr -El proc0
## 6.3. Docker
### 6.3.1. Basic Commands
Install docker:
https://docs.docker.com/engine/install/ubuntu/#installation-methods
Hello world doesn’t work at first – eventually it started working for me
```
sudo docker run -it docker/surprise
sudo docker ps -a
sudo docker ls
```


Networking:
https://docs.docker.com/network/network-tutorial-standalone/
```
sudo ip link set dev docker0 up/down
sudo docker exec -it iris ping -c 2 google.com
Sudo docker exec -it iris ip addr show
sudo docker network ls
sudo docker network inspect bridge
sudo docker network inspect host
sudo docker exec -it iris ls -la /usr/irissys/bin/libirisHLL.so
```

# 7. Unix tips
## 7.1. Why is computer slow?
* top
* free
* dstat
* iostat
## 7.2. Save terminal session with tmux
* sudo apt install tmux
* tmux
* export TERM=xterm-256color
* setenv TERM xterm-256color
* tmux ls
* tmux attach-session -t 0
* CTRL-B D  (detach)
# 8. Books
* (Stroustrop’s paper about C++ evolution) https://dl.acm.org/doi/abs/10.1145/3386320
* Fedor G Pikus Hands on Design Patterns with C++

# 9. Statistical Analysis and Machine Learning
## 9.1. Fundamentals of ML
* https://github.com/ageron/handson-ml2
## 9.2. Statistical measures in python:
```
popSD = numpy.std(population)

def stdDev(X):

    return variance(X)**0.5

pylab.array or numpy.array.mean()

r_squared:

def rSquared(observed, predicted):

    error = ((predicted - observed)**2).sum()

    meanError = error/len(observed)

    return 1 - (meanError/numpy.var(observed))

If the samples are not random and independent don’t make conclusions……

SEM:

def sem(popSD, sampleSize):

    return popSD/sampleSize**0.5

Gaussian: numpy.random.normal(mu, sigma, size)

Area in standard deviation:

     for numStd in (1, 1.96, 3):

        area = scipy.integrate.quad(gaussian,

                                    mu-numStd*sigma,

                                    mu+numStd*sigma,

                                    (mu, sigma))[0]

        print(' Fraction within', numStd,

              'std =', round(area, 4))
```
### 9.2.1. Plotting:
pylab.plot   pylab.hist   pylab.table
```
    pylab.errorbar(xVals, sizeMeans,

                   yerr = 1.96*pylab.array(sizeSDs), fmt = 'o',

                   label = '95% Confidence Interval')
```
### 9.2.2. Fitting:
```
def genFits(xVals, yVals, degrees):

    models = []

    for d in degrees:

        model = pylab.polyfit(xVals, yVals, d)

 

estYVals = pylab.polyval(model, xVals)
```
### 9.2.3. Sampling:
```
random.sample(population, sampleSize)

def leaveOneOut(examples, method, toPrint = True):

def split80_20(examples):

If the samples are not random and independent don’t make conclusions……

Survivor bias, non response bias, cherry picking
```
### 9.2.4. Learning
Clustering (kNearestNeighbor) is Unsupervised Learning
Classification (Logistic Regresion and k-means(greedy)) is Supervised Learning:
Logistic Regression comes in 2 kinds:

L1: many weights – the point is to drive weights to zero to avoid overfitting if 2 variables are correlated, only one will have weight

L2: spreads weight evenly across correlated variables

```
def minkowskiDist(v1, v2, p):

    """Assumes v1 and v2 are equal-length arrays of numbers

       Returns Minkowski distance of order p between v1 and v2"""

    dist = 0.0

    for i in range(len(v1)):

        dist += abs(v1[i] - v2[i])**p

    return dist**(1.0/p)

 
import sklearn.linear_model

def buildModel(examples, toPrint = True):

    featureVecs, labels = [],[]

    for e in examples:

        featureVecs.append(e.getFeatures())

        labels.append(e.getLabel())

    LogisticRegression = sklearn.linear_model.LogisticRegression

    model = LogisticRegression().fit(featureVecs, labels)

    if toPrint:

        print('model.classes_ =', model.classes_)

        for i in range(len(model.coef_)):

            print('For label', model.classes_[1])

            for j in range(len(model.coef_[0])):

                print('   ', Passenger.featureNames[j], '=',

                      model.coef_[0][j])

    return model
```

Reciever Operating Characteristic

auroc = sklearn.metrics.auc(xVals, yVals, True)

The Area Under Curve (AUC) metric measures the performance of a binary classification.

In a regression classification for a two-class problem using a probability algorithm, you will capture the probability threshold changes in an ROC curve.

Normally the threshold for two class is 0.5. Above this threshold, the algorithm classifies in one class and below in the other class.

Accuracy, Sensitivity, Specificity, Pos. Predictive Value, Neg Predictive Value
