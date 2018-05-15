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
    import urllib
except ImportError:
    import urllib.parse as urllib

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
    path = '/api/%s?%s' % (endpoint, urllib.urlencode(paramsdict))

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

            raw = ''

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

if args.input_file == None:
    # fetch a fresh test suite from osu api
    top_players = [
        124493, 4787150, 2558286, 1777162, 2831793, 50265
    ]

    osu = httplib.HTTPSConnection('osu.ppy.sh')

    for u in top_players:
        params = { 'u': u, 'limit': 100, 'type': 'id' }
        scores += osu_get(osu, 'get_user_best', params)

    params = { 'm': 0, 'since': '2015-11-26' }
    maps = osu_get(osu, 'get_beatmaps', params)

    for m in maps:
        params = { 'b': m['beatmap_id'] }
        map_scores = osu_get(osu, 'get_scores', params)

        if len(map_scores) == 0:
            sys.stderr.write('W: map has no scores???\n')
            continue

        # note: api also returns qualified and loved, so ignore
        # maps that don't have pp in rankings
        if not 'pp' in map_scores[0]:
            sys.stderr.write('W: ignoring loved/qualified map\n')
            continue

        for s in map_scores:
            s['beatmap_id'] = m['beatmap_id']

        scores += map_scores


    with open(args.output_file, 'w+') as f:
        f.write(json.dumps(scores))

else:
    # load existing test suite from json file
    with open(args.input_file, 'r') as f:
        scores = json.loads(f.read())


print('/* this code was automatically generated by gentest.py */')
print('')

# make code a little nicer by shortening mods
allmods = {
    'nf', 'ez', 'td', 'hd', 'hr', 'dt', 'ht', 'nc', 'fl', 'so', 'nomod'
}

for mod in allmods:
    print('#define %s MODS_%s' % (mod, mod.upper()))

print('''
struct score
{
    uint32_t id;
    uint16_t max_combo;
    uint16_t n300, n100, n50, nmiss;
    uint32_t mods;
    double pp;
};

struct score const suite[] =
{''')

seen_hashes = []

for s in scores:
    # why is every value returned by osu api a string?
    line = (
        '    { %s, %s, %s, %s, %s, %s, %s, %s },' %
        (
            s['beatmap_id'], s['maxcombo'], s['count300'],
            s['count100'], s['count50'], s['countmiss'],
            gen_modstr(int(s['enabled_mods'])), s['pp']
        )
    )

    # don't include identical scores by different people
    s = hashlib.sha1(line).digest()
    if s in seen_hashes:
        continue

    print(line)
    seen_hashes.append(s)

print('};\n')

for mod in allmods:
    print('#undef %s' % (mod))

