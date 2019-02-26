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
