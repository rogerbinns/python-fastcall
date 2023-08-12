#define PY_SSIZE_T_CLEAN
#include <Python.h>

/*
Note: there is deliberately no error checking because this is
exploratory code
*/

#define OBJ(o) (o ? o : Py_None)

static PyObject *
with_varargs_keywords(PyObject *self,
                      PyObject *args,
                      PyObject *kwargs)
{
    return Py_BuildValue("{s:O,s:O}", "args", OBJ(args), "kwargs", OBJ(kwargs));
}

static PyObject *
with_fastcall_keywords(PyObject *self,
                       PyObject *const *args,
                       Py_ssize_t nargs,
                       PyObject *kwnames)
{
    Py_ssize_t actual_nargs = nargs;
    if (kwnames)
        actual_nargs += PyTuple_Size(kwnames);
    PyObject *t = PyTuple_New(actual_nargs);
    for (Py_ssize_t i = 0; i < actual_nargs; i++)
    {
        Py_INCREF(args[i]);
        PyTuple_SetItem(t, i, args[i]);
    }
    return Py_BuildValue("{s:n,s:N,s:O}", "nargs", nargs, "args", t, "kwnames", OBJ(kwnames));
}

static PyObject *
bench_varargs_keywords(PyObject *self,
                       PyObject *args,
                       PyObject *kwargs)
{
    Py_RETURN_NONE;
}

static PyObject *
bench_fastcall_keywords(PyObject *self,
                        PyObject *const *args,
                        Py_ssize_t nargs,
                        PyObject *kwnames)
{
    Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
    {"with_varargs_keywords", (PyCFunction)with_varargs_keywords, METH_VARARGS | METH_KEYWORDS, NULL},
    {"with_fastcall_keywords", (PyCFunction)with_fastcall_keywords, METH_FASTCALL | METH_KEYWORDS, NULL},
    {"bench_varargs_keywords", (PyCFunction)bench_varargs_keywords, METH_VARARGS | METH_KEYWORDS, NULL},
    {"bench_fastcall_keywords", (PyCFunction)bench_fastcall_keywords, METH_FASTCALL | METH_KEYWORDS, NULL},
    {0, 0, 0, 0}};

static struct PyModuleDef calling_module = {PyModuleDef_HEAD_INIT, "calling", NULL, -1, methods};

PyMODINIT_FUNC
PyInit_calling(void)
{
    return PyModule_Create(&calling_module);
}