There are several `calling conventions
<https://docs.python.org/3/c-api/structures.html#c.PyMethodDef>`__
for CPython calling methods implemented in C.  This repository
provides more documentation and benchmarking for calling conventions
that take keywords.

.. contents::

Calling convention
==================

Calling :code:`method(1, 2, 3, four=4, five=5, six=6)``

varargs_keywords
----------------

This is the traditional approach that has been in Python forever.

.. code:: C

    with_varargs_keywords(PyObject *self,
                          PyObject *args,
                          PyObject *kwargs)

.. code:: python

    {'args': (1, 2, 3), 'kwargs': {'five': 5, 'four': 4, 'six': 6}}

A tuple for positional arguments and dict for keyword arguments.

fastcall_keywords
-----------------

There were a variety of internal to the interpreter calling conventions with names
like fastcall and vectorcall, ultimately exposed in this implementation for
C extensions.  `PEP 590 <https://peps.python.org/pep-0590/>`__  has details.

.. code:: C

    with_fastcall_keywords(PyObject *self,
                           PyObject *const *args,
                           Py_ssize_t nargs,
                           PyObject *kwnames)

.. code:: python

    {'args': (1, 2, 3, 4, 5, 6), 'kwnames': ('four', 'five', 'six'), 'nargs': 3}

`nargs` is the number of positional arguments passed. with `args` having those
plus a value for each keyword argument name in `kwnames`.


Parsing arguments
=================

Parsing is how you take the Python objects passed to the function and
make them available in the C code.

varargs_keywords
----------------

You use `PyArg_ParseTupleAndKeywords
<https://docs.python.org/3/c-api/arg.html?highlight=pyarg_parsetupleandkeywords#c.PyArg_ParseTupleAndKeywords>`__.
It takes a format string and addresses of the variables.

.. code:: c

    int errorcode;
    const char *message;

    static char *kwlist[] = {"errorcode", "message", NULL};
    /* i means int, s means const char * */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "is", kwlist, &errorcode, &message))
      return NULL;

fastcall_keywords
-----------------

CPython uses `argument clinic
<https://docs.python.org/3/howto/clinic.html>`__ to generate the C
code, but is not supported for use outside of CPython.  It generates
an intermediate function that has the fastcall signature, does the
parsing into variables, and then calls the actual C function
implementation passing those variable as regular C parameters.  It can
also do cleanup after the implementation returns such as releasing
buffer objects.

The generated code is as follows:

* Initialize C variables to default values
* Allocate a stack array the size of the parameter list - eg
  :code:`PyObject *fastargs[9]` if there are 9 parameters
* Call private internal :code:`_PyArg_UnpackKeywords`` which fills the
  array with the positional and keyword arguments into the appropriate
  slot.  It is passed the number of mandatory positional arguments,
  valid keyword parameter names and order etc, and does error checking
  that enough arguments were supplied etc.
* For each non-NULL value in fastargs call a convertor function as
  needed and update the corresponding C variable

Benchmarking
============

Is fastcall faster, especially when using keyword arguments?  I
benchmarked doing the calls with 3 positional parameters and 3 keyword
parameters, with the C method returning None.  ie there is no argument
parsing or any other work that is usually done.

.. list-table:: Times per call by Python version (nanoseconds)
    :widths: auto
    :header-rows: 1

    * - Python version
      - varargs
      - fastcall
    * - 3.12rc1
      - 158
      - 22
    * - 3.11.4
      - 156
      - 22
    * - 3.10.12
      - 139
      - 30
    * - 3.9.17
      - 140
      - 28
    * - 3.8.17
      - 130
      - 29
    * - 3.7.17
      - 113
      - 28


Each Python version was freshly compiled so the C compiler and
environment was the same across versions.  PGO was not used.

Questions
=========

**Q**: What happens if I provide duplicate keyword arguments like
:code:`method(1, arg=2, **{"arg": 3})`?

**A**:The Python runtime will detect duplicates and give a TypeError
along the lines of :code:`got multiple values for keyword argument
'arg'`.  It is not possible to call any function whether implemented
in C or Python with duplicate keyword arguments.

**Q**: What happens if I provide an argument positionally and as a
keyword?

**A**: For Python implemented methods you get :code:`TypeError: got
multiple values for argument 'arg'`.  C implemented methods need to
report the error themselves which PyArg_ParseTupleAndKeywords and
_PyArg_UnpackKeywords do.

**Q**: What about Python 3.6?  It is the base version in RHEL 8,
although updates are available.

**A**: The fastcall code will compile, but at runtime there is an
error.  :code:`SystemError: Bad call flags in PyCFunction_Call.
METH_OLDARGS is no longer supported!`

Irritations
===========

Helpful error messages
----------------------

If you call a function with a parameter that should be an integer, but
provide a string instead you get a TypeError.  Using varargs and
PyArg_ParseTupleAndKeywords the message will usually tell you which
argument it was and the name of the function (provided after a colon
in the format string).  This is especially helpful if there were
multiple parameters taking the type so you know which one it was.

If you use a `convertor
<https://docs.python.org/3/c-api/arg.html?highlight=pyarg_parsetupleandkeywords#other-objects>`__
then the convertor function doesn't know the argument name or number
and so can't give a helpful error text.

The argument clinic generated code does not include parameter name or
number information so you'll get the generic "must be a number not
str" with no idea which parameter is the problem.

Python 3.11 has an `add_note
<https://docs.python.org/3/library/exceptions.html#BaseException.add_note>`__
method on exceptions which would be an ideal place to include details
on which parameter was the problem.