osu! pp and difficulty calculator. automatically generated C bindings for
https://github.com/Francesco149/oppai-ng

usage
===========
.. code-block:: sh

    pip install oppai


.. code-block:: python

    #!/usr/bin/env python

    import sys
    from oppai import *

    ez = ezpp_new()
    ezpp(ez, sys.argv[1])
    print("%g pp" % ezpp_pp(ez))
    ezpp_free(ez)


.. code-block:: sh

    ./example.py /path/to/file.osu

.. code-block:: sh

    python -c 'help("oppai")'

for a list of functions, or just read the top of oppai.c for better doc


limitations
===========
for some reason, python3 doesn't provide a persisting pointer to strings
you pass to c code even if you aren't doing anything with them, so if you
want to reuse the handle at all you have to use ezpp_dup and ezpp_data_dup,
which create a copy of the strings you pass in. this is inefficient so
it's recommended to use autocalc mode and only call ezpp_dup or
ezpp_data_dup when you're actually changing map
