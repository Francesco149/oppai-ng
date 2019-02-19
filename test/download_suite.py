#!/usr/bin/env python

# very rough script that downloads unique maps from test_suite.json that
# gensuite.py generates

import sys
import json

try:
    import httplib
except ImportError:
    import http.client as httplib

try:
    import urllib
except ImportError:
    import urllib.parse as urllib

osu = httplib.HTTPSConnection('osu.ppy.sh')

def osu_get(path):
    while True:
        try:
            osu.request('GET', path)
            r = osu.getresponse()

            raw = bytes()

            while True:
                try:
                    raw += r.read()
                    break
                except httplib.IncompleteRead as e:
                    raw += e.partial

            return raw

        except (httplib.HTTPException, ValueError) as e:
            sys.stderr.write('%s\n' % (traceback.format_exc()))

            # prevents exceptions on next request if the
            # response wasn't previously read due to errors
            try:
                osu.getresponse().read()
            except httplib.HTTPException:
                pass

            time.sleep(5)


if len(sys.argv) != 2:
    sys.stderr.write('usage: %s test_suite.json\n' % sys.argv[0])
    sys.exit(1)

with open(sys.argv[1], 'r') as f:
    scores = json.loads(f.read())

unique_maps = set([s['beatmap_id'] for m in [0, 1] for s in scores[m]])
i = 1

for b in unique_maps:
    sys.stderr.write(
        "[%.02f%% - %d/%d] %s" % (i / float(len(unique_maps)) * 100, i,
            len(unique_maps), b)
    )
    i += 1

    # TODO: tmp file and rename
    try:
        with open(b + '.osu', 'r') as f:
            sys.stderr.write(' (already exists)\n')
            continue
    except FileNotFoundError:
        pass

    sys.stderr.write('\n')

    with open(b + '.osu', 'wb') as f:
        f.write(osu_get('/osu/' + b))
