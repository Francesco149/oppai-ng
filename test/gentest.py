#!/usr/bin/env python

import sys
import os
import time
import json
import traceback
import argparse
import hashlib

if sys.version_info[0] < 3:
  # hack to force utf-8
  reload(sys)
  sys.setdefaultencoding('utf-8')

try:
  import httplib
except ImportError:
  import http.client as httplib

try:
  from urllib import urlencode
except ImportError:
  from urllib.parse import urlencode

# -------------------------------------------------------------------------

parser = argparse.ArgumentParser(
  description = (
    'generates the oppai test suite. outputs c++ code to ' +
    'stdout and the json dump to a file.'
  )
)

parser.add_argument(
  '-key',
  default = None,
  help = (
    'osu! api key. required if -input-file is not present. ' +
    'can also be specified through the OSU_API_KEY ' +
    'environment variable'
  )
)

parser.add_argument(
  '-output-file',
  default = 'test_suite.json',
  help = 'dumps json to this file'
)

parser.add_argument(
  '-input-file',
  default = None,
  help = (
    'loads test suite from this json file instead of '
    'fetching it from osu api. if set to "-", json will be '
    'read from standard input'
  )
)

args = parser.parse_args()

if args.key == None and 'OSU_API_KEY' in os.environ:
  args.key = os.environ['OSU_API_KEY']

# -------------------------------------------------------------------------

osu_treset = time.time() + 60
osu_ncalls = 0

def osu_get(conn, endpoint, paramsdict=None):
  # GETs /api/endpoint?paramsdict&k=args.key from conn
  # return json object, exits process on api errors
  global osu_treset, osu_ncalls, args

  sys.stderr.write('%s %s\n' % (endpoint, str(paramsdict)))

  paramsdict['k'] = args.key
  path = '/api/%s?%s' % (endpoint, urlencode(paramsdict))

  while True:
    while True:
      if time.time() >= osu_treset:
        osu_ncalls = 0
        osu_treset = time.time() + 60
        sys.stderr.write('\napi ready\n')

      if osu_ncalls < 60:
        break
      else:
        sys.stderr.write('waiting for api cooldown...\r')
        time.sleep(1)


    try:
      conn.request('GET', path)
      osu_ncalls += 1
      r = conn.getresponse()

      raw = b''

      while True:
        try:
          raw += r.read()
          break
        except httplib.IncompleteRead as e:
          raw += e.partial

      j = json.loads(raw)

      if 'error' in j:
        sys.stderr.write('%s\n' % j['error'])
        sys.exit(1)

      return j

    except (httplib.HTTPException, ValueError) as e:
      sys.stderr.write('%s\n' % (traceback.format_exc()))

      try:
        # prevents exceptions on next request if the
        # response wasn't previously read due to errors
        conn.getresponse().read()

      except httplib.HTTPException:
        pass

      time.sleep(5)


def gen_modstr(bitmask):
  # generates c++ code for a mod combination's bitmask
  mods = []

  allmods = {
    (1<< 0, 'nf'), (1<< 1, 'ez'), (1<< 2, 'td'), (1<< 3, 'hd'),
    (1<< 4, 'hr'), (1<< 6, 'dt'), (1<< 8, 'ht'),
    (1<< 9, 'nc'), (1<<10, 'fl'), (1<<12, 'so')
  }

  for bit, string in allmods:
    if bitmask & bit != 0:
      mods.append(string)

  if len(mods) == 0:
    return 'nomod'

  return ' | '.join(mods)

# -------------------------------------------------------------------------

if args.key == None:
  sys.stderr.write(
    'please set OSU_API_KEY or pass it as a parameter\n'
  )
  sys.exit(1)


scores = []

#top_players = [
#  [ 124493, 4787150, 2558286, 1777162, 2831793, 50265 ],
#  [ 3174184, 8276884, 5991961, 2774767 ]
#]

top_players = [
  [ 4504101, 7562902, 6447454, 4787150, 11367222, 5339515, 8179335, 4196808, 4650315 ]
]

if args.input_file == None:
  # fetch a fresh test suite from osu api
  osu = httplib.HTTPSConnection('osu.ppy.sh')

  for m, ids in enumerate(top_players):
    for u in ids:
      params = { 'u': u, 'limit': 100, 'type': 'id', 'm': m }
      batch = osu_get(osu, 'get_user_best', params)
      for s in batch:
        s['mode'] = m
      scores += batch

  # TODO: uncomment when all scores are properly recalced
  #params = { 'm': 0, 'since': '2019-01-01' }
  #maps = osu_get(osu, 'get_beatmaps', params)

  #for m in maps:
  #  mode = 0
  #  params = { 'b': m['beatmap_id'], 'm': mode }
  #  map_scores = osu_get(osu, 'get_scores', params)

  #  if len(map_scores) == 0:
  #    sys.stderr.write('W: map has no scores???\n')
  #    continue

  #  # note: api also returns qualified and loved, so ignore
  #  # maps that don't have pp in rankings
  #  if not 'pp' in map_scores[0] or map_scores[0]['pp'] is None:
  #    sys.stderr.write('W: ignoring loved/qualified map\n')
  #    continue

  #  for s in map_scores:
  #    s['beatmap_id'] = m['beatmap_id']
  #    s['mode'] = mode

  #  scores += map_scores


  with open(args.output_file, 'w+') as f:
    f.write(json.dumps(scores))

else:
  # load existing test suite from json file
  with open(args.input_file, 'r') as f:
    scores = json.loads(f.read())
  # sort by mode by map
  scores.sort(key=lambda x: int(x['beatmap_id'])<<32|x['mode'],
    reverse=True)


print('/* this code was automatically generated by gentest.py */')
print('')

# make code a little nicer by shortening mods
allmods = {
  'nf', 'ez', 'td', 'hd', 'hr', 'dt', 'ht', 'nc', 'fl', 'so', 'nomod'
}

for mod in allmods:
  print('#define %s MODS_%s' % (mod, mod.upper()))

print('''
typedef struct {
  int mode;
  int id;
  int max_combo;
  int n300, n100, n50, nmiss;
  int mods;
  double pp;
} score_t;

score_t suite[] = {''')

seen_hashes = []

for s in scores:
  # due to tiny floating point errors, maps can even double
  # in combo and not even lazer gets this right, taiko converts are hell
  # so I'm just gonna exclude them
  if s['mode'] == 1:
    is_convert = False
    with open('test_suite/'+s['beatmap_id']+'.osu') as f:
      for line in f:
        split = line.split(':')
        if len(split) >= 2 and split[0] == 'Mode' and int(split[1]) == 0:
          is_convert = True
          break

    if is_convert:
        continue

    # some taiko maps ignore combo scaling for no apparent reason
    # so i will only include full combos for now
    if int(s['countmiss']) != 0:
        continue

  if s['pp'] is None:
    continue

  # why is every value returned by osu api a string?
  line = (
    '  { %d, %s, %s, %s, %s, %s, %s, %s, %s },' %
    (
      s['mode'], s['beatmap_id'], s['maxcombo'], s['count300'],
      s['count100'], s['count50'], s['countmiss'],
      gen_modstr(int(s['enabled_mods'])), s['pp']
    )
  )

  # don't include identical scores by different people
  s = hashlib.sha1(line.encode('utf-8')).digest()
  if s in seen_hashes:
    continue

  print(line)
  seen_hashes.append(s)

print('};\n')

for mod in allmods:
  print('#undef %s' % (mod))

