%module oppai
%feature("autodoc", "3");
%apply int *OUTPUT {int*}
%{
#define OPPAI_IMPLEMENTATION
#include "oppai.c"
%}
#include "oppai.c"
