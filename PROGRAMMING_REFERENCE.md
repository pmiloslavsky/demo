Table of Contents:
- [C/C++](#cc)
  - [API Reference](#api-reference)
  - [Compiler Version Supported Features](#compiler-version-supported-features)
  - [C++ Libraries](#c-libraries)
    - [GNU libstdc++](#gnu-libstdc)
    - [List of Libraries](#list-of-libraries)
    - [GUI](#gui)
    - [Intel TBB](#intel-tbb)
  - [Replacing STL allocators:](#replacing-stl-allocators)
  - [Cheat sheets](#cheat-sheets)
- [Python](#python)
- [Algorithms](#algorithms)
  - [Reference](#reference)
  - [Interviewing and Competitive Programming](#interviewing-and-competitive-programming)
- [Design Patterns and OO](#design-patterns-and-oo)
  - [UML syntax](#uml-syntax)
- [Python](#python-1)
  - [Tutorials](#tutorials)
  - [Cheat Sheets](#cheat-sheets-1)
  - [profiling](#profiling)
  - [Source Code:](#source-code)
- [Debugging,Analysis and Performance Tuning](#debugginganalysis-and-performance-tuning)
  - [GDB](#gdb)
  - [Profiling](#profiling-1)
- [Unix tips](#unix-tips)
- [Books](#books)
- [Statistical Analysis and Learning](#statistical-analysis-and-learning)
    - [Statistical measures:](#statistical-measures)
    - [Plotting:](#plotting)
    - [Fitting:](#fitting)
    - [Sampling:](#sampling)
    - [Learning](#learning)

# C/C++
## API Reference
 * CPP: https://en.cppreference.com/w/cpp
 * CPP: https://www.cplusplus.com/reference/unordered_map/unordered_map/
 * CPP How to Program: https://cs.fit.edu/~mmahoney/cse2050/how2cpp.html
 * C standard library: https://www.cplusplus.com/reference/clibrary/
## Compiler Version Supported Features
 * https://en.cppreference.com/w/cpp/compiler_support
## C++ Libraries
### GNU libstdc++
* Docs: https://gcc.gnu.org/onlinedocs/libstdc++/manual/index.html
* Library test suite for parallel sort: https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/testsuite/25_algorithms/pstl/alg_sorting/sort.cc;h=e0c55df404f82dc360cea77ce411511bc2b4e619;hb=HEAD
### List of Libraries
https://en.cppreference.com/w/cpp/links/libs
### GUI
* SFML https://www.sfml-dev.org/index.php
* TGUI https://tgui.eu/
### Intel TBB
Forms the basis of parallel algorithms in latest C++ versions

https://software.intel.com/content/www/us/en/develop/documentation/tbb-documentation/top.html

Introduction: https://www.youtube.com/watch?v=9Otq_fcUnPE

## Replacing STL allocators:
* https://gcc.gnu.org/onlinedocs/libstdc++/manual/memory.html
* https://johnysswlab.com/the-price-of-dynamic-memory-allocation/
* 
## Cheat sheets
 * Syntax cheat sheet: https://github.com/gibsjose/cpp-cheat-sheet/blob/master/C%2B%2B%20Syntax.md
 * Data Structures and Algorithms cheat sheet: https://github.com/gibsjose/cpp-cheat-sheet/blob/master/Data%20Structures%20and%20Algorithms.md
 * C++ STL quick reference: https://overapi.com/static/cs/STL%20Quick%20Reference%201.29.pdf
 * 
# Python
# Algorithms
## Reference
https://www.geeksforgeeks.org/
## Interviewing and Competitive Programming
* https://www.hackerrank.com/
* https://www.hackerrank.com/philip_miloslav1
# Design Patterns and OO
* SOLID:  https://scotch.io/bar-talk/s-o-l-i-d-the-first-five-principles-of-object-oriented-design
* Catalog: https://en.wikipedia.org/wiki/Software_design_pattern
* Cloud Patterns: https://docs.microsoft.com/en-us/azure/architecture/patterns/
* UML: 
## UML syntax
<<interface>>  +public  -private    variable: type

Inheritance: from class: solid line black arrow, from abstract class: solid line white arrow, from interface: dashed line white arrow

Aggregation: white diamond

Composition: black diamond
# Python
## Tutorials
* https://realpython.com/
## Cheat Sheets
* https://gto76.github.io/python-cheatsheet/ 
## profiling
python -m cProfile -s time circuit.py
## Source Code:
# Debugging,Analysis and Performance Tuning
## GDB
* SIGSEGV: http://unknownroad.com/rtfm/gdbtut/gdbsegfault.html
## Profiling
# Unix tips
# Books
* (Stroustrop’s paper about C++ evolution) https://dl.acm.org/doi/abs/10.1145/3386320
* Fedor G Pikus Hands on Design Patterns with C++

# Statistical Analysis and Learning
### Statistical measures:
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
### Plotting:
pylab.plot   pylab.hist   pylab.table
```
    pylab.errorbar(xVals, sizeMeans,

                   yerr = 1.96*pylab.array(sizeSDs), fmt = 'o',

                   label = '95% Confidence Interval')
```
### Fitting:
```
def genFits(xVals, yVals, degrees):

    models = []

    for d in degrees:

        model = pylab.polyfit(xVals, yVals, d)

 

estYVals = pylab.polyval(model, xVals)
```
### Sampling:
```
random.sample(population, sampleSize)

def leaveOneOut(examples, method, toPrint = True):

def split80_20(examples):

If the samples are not random and independent don’t make conclusions……

Survivor bias, non response bias, cherry picking
```
### Learning
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
