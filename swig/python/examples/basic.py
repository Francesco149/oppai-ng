#!/usr/bin/env python

import sys
from oppai import *

ez = ezpp_new()
ezpp(ez, sys.argv[1])
print("%g pp" % ezpp_pp(ez))
ezpp_free(ez)
