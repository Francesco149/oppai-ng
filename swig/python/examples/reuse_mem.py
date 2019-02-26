#!/usr/bin/env python

import sys
from oppai import *

if sys.version_info[0] < 3:
  # hack to force utf-8 on py < 3
  reload(sys)
  sys.setdefaultencoding("utf-8")

def mods_str(mods):
  mods_str = "+"
  if mods == 0:
    mods_str += "nomod"
  else:
    if mods & MODS_HD: mods_str += "hd"
    if mods & MODS_DT: mods_str += "dt"
    if mods & MODS_HR: mods_str += "hr"
  return mods_str

ez = ezpp_new()
ezpp_set_autocalc(ez, 1)
for osufile in sys.argv[1:]:
  # by providing the map in memory we can speed up subsequent re-parses
  f = open(osufile, 'r')
  data = f.read()
  f.close()
  ezpp_data_dup(ez, data, len(data.encode('utf-8')))
  print("%s - %s [%s]" % (ezpp_artist(ez), ezpp_title(ez), ezpp_version(ez)))
  print("%g stars" % ezpp_stars(ez))
  for mods in [ 0, MODS_HR, MODS_HD | MODS_HR, MODS_DT, MODS_HD | MODS_DT ]:
    print(mods_str(mods))
    ezpp_set_mods(ez, mods)
    for acc in range(95, 101):
      ezpp_set_accuracy_percent(ez, acc)
      print("%g%% -> %g pp" % (acc, ezpp_pp(ez)))
  print("")
ezpp_free(ez)
