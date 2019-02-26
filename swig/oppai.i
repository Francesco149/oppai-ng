%module oppai
%feature("autodoc", "3");
%apply int *OUTPUT {int*}
%begin{
#define SWIG_PYTHON_2_UNICODE
}
%{
#define OPPAI_IMPLEMENTATION
#include "oppai.c"
%}
#include "oppai.c"
