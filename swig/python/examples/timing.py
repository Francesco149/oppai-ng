#!/usr/bin/env python

import sys
from oppai import *

# prints timing points (just a test for this interface)
ez = ezpp_new()
ezpp(ez, sys.argv[1])
for i in range(ezpp_ntiming_points(ez)):
  time = ezpp_timing_time(ez, i)
  ms_per_beat = ezpp_timing_ms_per_beat(ez, i)
  change = ezpp_timing_change(ez, i)
  print("%f | %f beats per ms | change: %d" % (time, ms_per_beat, change))
ezpp_free(ez)
