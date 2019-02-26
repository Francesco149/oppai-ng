import os
import sys

try:
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup, Extension

try:
  from oppai import oppai_version_str
except Exception:
  def oppai_version_str():
    return "INVALID"

oppai_classifiers = [
    "Programming Language :: Python :: 2",
    "Programming Language :: Python :: 3",
    "Intended Audience :: Developers",
    "License :: Public Domain",
    "Topic :: Software Development :: Libraries",
    "Topic :: Utilities",
]

f = open("README.rst", "r")
oppai_readme = f.read()
f.close()

oppai_sources=['oppai.i']
if os.system('swig') != 0:
  oppai_sources=['oppai_wrap.c', 'oppai.c']

setup(
    name="oppai",
    version=oppai_version_str(),
    author="Franc[e]sco",
    author_email="lolisamurai@tfwno.gf",
    url="https://github.com/Francesco149/oppai-ng",
    ext_modules=[Extension('_oppai', oppai_sources)],
    py_modules=["oppai"],
    description="osu! pp and difficulty calculator, C bindings",
    long_description=oppai_readme,
    license="Unlicense",
    classifiers=oppai_classifiers,
    keywords="osu! osu"
)
