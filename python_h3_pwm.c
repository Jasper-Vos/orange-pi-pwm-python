#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>

#define SW_PORTC_IO_BASE 0x01c20800 // dataregister
#define PWM_CH_CTRL 0x01c21400      // pwm control register

int frequency;

static PyObject *pwm_control(PyObject *self, PyObject *args){
    int duty_cycle;

    if (!PyArg_ParseTuple(args, "i", &duty_cycle))
    {
        return NULL;
    }
    //TODO: Throw error.
    if(duty_cycle > 100) {
        return NULL;
    }
    unsigned int *pc;
    int fd;
    char *ptr;
    unsigned int addr_start, addr_offset, PageSize, PageMask, data;

    PageSize = sysconf(_SC_PAGESIZE);
    PageMask = (~(PageSize - 1));
    addr_start = SW_PORTC_IO_BASE & PageMask;
    addr_offset = SW_PORTC_IO_BASE & ~PageMask;

    fd = open("/dev/mem", O_RDWR);
    if (fd < 0)
    {
        // TODO: Throw error.
        perror("Unable to open /dev/mem");
        return NULL;
    }

    pc = mmap(0, PageSize * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr_start);

    if (pc == MAP_FAILED)
    {
        // TODO: Throw error.
        perror("Unable to mmap file");
        printf("pc:%lx\n", (unsigned long)pc);
        return NULL;
    }
    ptr = (char *)pc + addr_offset;
    data = *(unsigned int *)(ptr + 0x00); // offset for controller register
    // PA_CFGO_REG  0x01c20800 page 317 H3 datasheet
    data |= (1 << 20); // set port PA5 to pwm (011)
    data |= (1 << 21);
    data &= ~(1 << 22);
    *(unsigned int *)(ptr + 0x00) = data;
    // PWM control register
    PageSize = sysconf(_SC_PAGESIZE);
    PageMask = (~(PageSize - 1));
    addr_start = PWM_CH_CTRL & PageMask;
    addr_offset = PWM_CH_CTRL & ~PageMask;

    pc = mmap(0, PageSize * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr_start);

    if (pc == MAP_FAILED)
    {
        // TODO: Throw error.
        perror("Unable to mmap file");
        printf("pc:%lx\n", (unsigned long)pc);
        return NULL;
    }
    ptr = (char *)pc + addr_offset;
    data = *(unsigned int *)(ptr + 0x00); // offset for controller register
    //       data |= (1<<0);                              //prescale = 360
    //      data |= (1<<1);                              //

    //       data |= (1<<2);                              //prescale = 72K
    //      data |= (1<<3);                              //prescal

    data |= (1 << 4); // pwm channel 0 enable
    data |= (1 << 5); // low - high level
    data |= (1 << 6); // clock gating
    //       data |= (1<<7);                              //cycle mode / pulse mode
    data |= (1 << 8); // control register
    //       data |= (1<<9);                              //pwm_bypass 24Mhz 1 is enable
    *(unsigned int *)(ptr + 0x00) = data;

    // PWM_CH0_PERIOD PWM CHANNEL 0 Period Register offset 0x04
    ptr = (char *)pc + addr_offset + 0x04;

    data = (0x27100000) + duty_cycle * 100; // Frequency of 2.4 kHz 
    *(unsigned int *)(ptr + 0x00) = data;

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
    PyObject *module = PyModule_Create(&pwmmodule);
    PyObject *v = PyLong_FromSize_t(0x0000);
    PyObject_SetAttrString(module, "PRESCALAR_120", v);
    Py_DECREF(v);
    v = PyLong_FromSize_t(0x0001);
    PyObject_SetAttrString(module, "PRESCALAR_180", v);
    Py_DECREF(v);
    v = PyLong_FromSize_t(0x0002);
    PyObject_SetAttrString(module, "PRESCALAR_240", v);
    Py_DECREF(v);
    v = PyLong_FromSize_t(0x0003);
    PyObject_SetAttrString(module, "PRESCALAR_360", v);
    Py_DECREF(v);
    v = PyLong_FromSize_t(0x0004);
    PyObject_SetAttrString(module, "PRESCALAR_480", v);
    Py_DECREF(v);
    v = PyLong_FromSize_t(0x0008);
    PyObject_SetAttrString(module, "PRESCALAR_12K", v);
    Py_DECREF(v);
    v = PyLong_FromSize_t(0x0009);
    PyObject_SetAttrString(module, "PRESCALAR_24K", v);
    Py_DECREF(v);
    v = PyLong_FromSize_t(0x000A);
    PyObject_SetAttrString(module, "PRESCALAR_36K", v);
    Py_DECREF(v);
    v = PyLong_FromSize_t(0x000B);
    PyObject_SetAttrString(module, "PRESCALAR_48K", v);
    Py_DECREF(v);
    v = PyLong_FromSize_t(0x000C);
    PyObject_SetAttrString(module, "PRESCALAR_72K", v);
    Py_DECREF(v);
    v = PyLong_FromSize_t(0x000F);
    PyObject_SetAttrString(module, "PRESCALAR_1", v);
    Py_DECREF(v);
    return module;
}