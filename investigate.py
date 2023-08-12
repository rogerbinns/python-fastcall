#!/usr/bin/env python

import pprint
import timeit

import calling # python3 setup.py build_ext --inplace --force

print("Calling convention for meth(1, 2, 3, four=4, five=5, six=6)")
for n in dir(calling):
    if n.startswith("with_"):
        v=getattr(calling, n)(1, 2, 3, four=4, five=5, six=6)
        print(n)
        pprint.pprint(v)

print("Duplicate keyword args")
for n in dir(calling):
    if n.startswith("with_"):
        print(n)
        try:
            v=getattr(calling, n)(1, 2, 3, four=4, five=5, **{"four": 6})
            pprint.pprint(v)
        except TypeError:
            print("  TypeError")

print("Calling overhead method(1, 2, 3, four=4, five=5, six=6)")
for n in dir(calling):
    if n.startswith("bench_"):
        print(n)
        t=timeit.Timer("method(1, 2, 3, four=4, five=5, six=6)", setup=f"method=getattr(calling, '{ n }')", globals={"calling": calling})
        nloops, _ = t.autorange()
        nloops *=5 # run for ~1 second
        took=t.timeit(nloops)
        print(f"{ 1e9 * took / nloops:.1f} nanoseconds per call")

