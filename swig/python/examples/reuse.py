#!/usr/bin/env python

import sys
from oppai import *

ez = ezpp_new()
ezpp_set_autocalc(ez, 1)
for osufile in sys.argv[1:]:
  ezpp_dup(ez, osufile)
  print("%s - %s [%s]" % (ezpp_artist(ez), ezpp_title(ez), ezpp_version(ez)))
  print("%g stars" % ezpp_stars(ez))
  for acc in range(95, 101):
    ezpp_set_accuracy_percent(ez, acc)
    print("%g%% -> %g pp" % (acc, ezpp_pp(ez)))
  print("")
ezpp_free(ez)
