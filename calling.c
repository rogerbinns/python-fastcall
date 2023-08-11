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
    return Py_BuildValue("OO", OBJ(args), OBJ(kwargs));
}

static PyObject *
with_fastcall_keywords(PyObject *self,
                       PyObject *const *args,
                       Py_ssize_t nargs,
                       PyObject *kwnames)
{
    nargs += 3;
    PyObject *t = PyTuple_New(nargs);
    for (Py_ssize_t i = 0; i < nargs; i++)
    {
        Py_INCREF(args[i]);
        PyTuple_SetItem(t, i, args[i]);
    }
    return Py_BuildValue("NO", t, OBJ(kwnames));
}

static PyMethodDef methods[] = {
    {"with_varargs_keywords", (PyCFunction)with_varargs_keywords, METH_VARARGS | METH_KEYWORDS, NULL},
    {"with_fastcall_keywords", (PyCFunction)with_fastcall_keywords, METH_FASTCALL | METH_KEYWORDS, NULL},
    {0, 0, 0, 0}};

static struct PyModuleDef calling_module = {PyModuleDef_HEAD_INIT, "calling", NULL, -1, methods};

PyMODINIT_FUNC
PyInit_calling(void)
{
    return PyModule_Create(&calling_module);
}