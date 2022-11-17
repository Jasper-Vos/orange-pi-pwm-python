#define PY_SSIZE_T_CLEAN
#include <Python.h>


int frequency;
int duty_cycle;

static PyObject *pwm_control(PyObject *self, PyObject *args) {
    int *freq, *duty_cylce;

    if(!PyArg_ParseTuple(args, "ii", &freq, &duty_cylce)){
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef pwmMethods[] = {
    {"pwm", pwm_control, METH_VARARGS, "Python PWM interface for Allwinner H3 based computer"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef pwmmodule = {
    PyModuleDef_HEAD_INIT,
    "H3pwm",
    "Python PWM interface for Allwinner H3 based computer",
    -1,
    pwmMethods};

PyMODINIT_FUNC PyInit_h3pwm(void)
{
    return PyModule_Create(&pwmmodule);
}