#!/usr/bin/env python

import pprint

import calling # python3 setup.py build_ext --inplace --force

for n in dir(calling):
    if n.startswith("with_"):
        v=getattr(calling, n)(1, 2, 3, four=4, five=5, six=6)
        print(n)
        pprint.pprint(v)