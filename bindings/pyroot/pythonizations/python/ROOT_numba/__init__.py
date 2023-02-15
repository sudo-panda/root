import sys

if sys.version_info < (3, 7):
    raise Exception("ROOT numba extension requires Python 3.7 or higher")

import numba

if not hasattr(numba, 'version_info') or numba.version_info < (0, 54):
    raise Exception("ROOT numba extension requires Numba version 0.54 or higher")

import cppyy.numba_ext