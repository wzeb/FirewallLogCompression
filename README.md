Introduction
------------

Compress Firwall logs, and construct indexs to speed up retrieval.

version : v 1.0

Only implemented the core code and basic functions required for the experiment


Compatibility
-------------

    
    

Installation and usage
----------------------

build:

    make clean
    make


To test compression, run:

    ./main store

To test retrieval, run:

    ./main retrieval

Dependent environment :

    yaml-cpp [https://github.com/jbeder/yaml-cpp]
    c++ 11


