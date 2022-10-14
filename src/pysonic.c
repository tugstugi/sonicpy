//
// https://stackabuse.com/enhancing-python-with-custom-c-extensions/
// https://www.geeksforgeeks.org/python-opaque-pointers-in-c-extension-modules/
//
#define PY_SSIZE_T_CLEAN 1

#include <stdio.h>
#include "Python.h"
#include "sonic.h"

#define READ_BUFFER_SIZE 22050

// Definition of the sonic object
typedef struct {
    PyObject_HEAD
    sonicStream stream;
} py_sonic; /* Sonic */

#define PY_SONIC(x) ((py_sonic *) x)

extern PyTypeObject py_sonic_t;

static PyObject* py_sonic_getattr(PyObject* self, char* name) {
    return PyObject_GenericGetAttr((PyObject *)self, PyUnicode_FromString(name));
}

int py_sonic_init(PyObject* self, PyObject* args, PyObject *kwds) {
    py_sonic* ps = NULL;
    int sampleRate, channels;

    // Needs to be given the sampling rate and number of channels
    if (!PyArg_ParseTuple(args, "II:Sonic", &sampleRate, &channels)) {
        PyErr_SetString(PyExc_RuntimeError, "Requires sampling rate and number of channels");
        return 0;
    }
    if (channels != 1) {
        PyErr_SetString(PyExc_TypeError, "channels should be 1");
        return 0;
    }
    // Create the object
    ps = PY_SONIC(self);
    ps->stream = sonicCreateStream(sampleRate, channels);

    return 1;
}


static void py_sonic_dealloc(PyObject* self, PyObject* args) {
    py_sonic* ps = PY_SONIC(self);
    if (ps->stream) {
        sonicDestroyStream(ps->stream);
        ps->stream = NULL;
    }
    PyObject_DEL(self);
}

static PyObject* py_sonic_set_speed(PyObject* self, PyObject* args) {
    float speed;
    if (! PyArg_ParseTuple(args, "f", &speed)) { return NULL; }
    if (speed < 0.2 || speed > 6.0) {
        PyErr_SetString(PyExc_TypeError, "speed should be between 0.2 and 6.0");
        return NULL;
    }
    sonicSetSpeed(PY_SONIC(self)->stream, speed);
    Py_INCREF(Py_None);
    return Py_None;
}
static PyObject* py_sonic_get_speed(PyObject* self, PyObject* args) {
    return Py_BuildValue("f", sonicGetSpeed(PY_SONIC(self)->stream));
}

static PyObject* py_sonic_set_pitch(PyObject* self, PyObject* args) {
    float pitch;
    if (! PyArg_ParseTuple(args, "f", &pitch)) { return NULL; }
    if (pitch < 0.2 || pitch > 6.0) {
        PyErr_SetString(PyExc_TypeError, "pitch should be between 0.2 and 6.0");
        return NULL;
    }
    sonicSetPitch(PY_SONIC(self)->stream, pitch);
    Py_INCREF(Py_None);
    return Py_None;
}
static PyObject* py_sonic_get_pitch(PyObject* self, PyObject* args) {
    return Py_BuildValue("f", sonicGetPitch(PY_SONIC(self)->stream));
}

static PyObject* py_sonic_set_volume(PyObject* self, PyObject* args) {
    float volume;
    if (! PyArg_ParseTuple(args, "f", &volume)) { return NULL; }
    if (volume < 0.0 || volume > 2.0) {
        PyErr_SetString(PyExc_TypeError, "volume should be between 0.0 and 2.0");
        return NULL;
    }
    sonicSetVolume(PY_SONIC(self)->stream, volume);
    Py_INCREF(Py_None);
    return Py_None;
}
static PyObject* py_sonic_get_volume(PyObject* self, PyObject* args) {
    return Py_BuildValue("f", sonicGetVolume(PY_SONIC(self)->stream));
}

static PyObject* py_sonic_set_quality(PyObject* self, PyObject* args) {
    int quality;
    if (! PyArg_ParseTuple(args, "i", &quality)) { return NULL; }
    if (quality != 0 && quality != 1) {
        PyErr_SetString(PyExc_TypeError, "quality should be either 0 or 1");
        return NULL;
    }
    sonicSetQuality(PY_SONIC(self)->stream, quality);
    Py_INCREF(Py_None);
    return Py_None;
}
static PyObject* py_sonic_get_quality(PyObject* self, PyObject* args) {
    return Py_BuildValue("i", sonicGetQuality(PY_SONIC(self)->stream));
}


static PyObject* py_sonic_write_short(PyObject* self, PyObject* args) {
    short *buffer;
    Py_ssize_t buffer_length = 0;
    if (! PyArg_ParseTuple(args, "y#", &buffer, &buffer_length)) { return NULL; }
    buffer_length = buffer_length / 2;  // bytes to shorts
    return PyBool_FromLong(sonicWriteShortToStream(PY_SONIC(self)->stream, buffer, buffer_length));
}

static PyObject* py_sonic_read_short(PyObject* self, PyObject* args) {
    short buffer[READ_BUFFER_SIZE];
    int maxSamples = 0;
    if (! PyArg_ParseTuple(args, "i", &maxSamples)) { return NULL; }
    if (maxSamples < 1 || maxSamples > READ_BUFFER_SIZE) {
        PyErr_SetString(PyExc_TypeError, "maxSamples to be read should be between 1 and 22050");
        return NULL;
    }
    int count = sonicReadShortFromStream(PY_SONIC(self)->stream, buffer, maxSamples);
    return PyBytes_FromStringAndSize((const char *) buffer, 2 * count);
}

static PyObject* py_sonic_samples_available(PyObject* self, PyObject* args) {
    return Py_BuildValue("i", sonicSamplesAvailable(PY_SONIC(self)->stream));
}

static PyObject* py_sonic_flush(PyObject* self, PyObject* args) {
    return PyBool_FromLong(sonicFlushStream(PY_SONIC(self)->stream));
}

static PyMethodDef sonic_methods[] = {
    { "set_speed", py_sonic_set_speed, METH_VARARGS, "" },
    { "get_speed", py_sonic_get_speed, METH_NOARGS, "" },
    { "set_pitch", py_sonic_set_pitch, METH_VARARGS, "" },
    { "get_pitch", py_sonic_get_pitch, METH_NOARGS, "" },
    { "set_volume", py_sonic_set_volume, METH_VARARGS, "" },
    { "get_volume", py_sonic_get_volume, METH_NOARGS, "" },
    { "set_quality", py_sonic_set_quality, METH_VARARGS, "" },
    { "get_quality", py_sonic_get_quality, METH_NOARGS, "" },

    { "write_short", py_sonic_write_short, METH_VARARGS, "" },
    { "read_short", py_sonic_read_short, METH_VARARGS, "" },
    { "samples_available", py_sonic_samples_available, METH_NOARGS, "" },
    { "flush", py_sonic_flush, METH_NOARGS, "" },
    { NULL }
};

PyTypeObject py_sonic_t = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Sonic",                        /* tp_name */
    sizeof(py_sonic),               /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor) py_sonic_dealloc,  /* tp_dealloc */
    0,                              /* tp_vectorcall_offset */
    (getattrfunc) py_sonic_getattr, /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_as_async */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    "Sonic",                        /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    sonic_methods,                  /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    py_sonic_init,                  /* tp_init */
    0,                              /* tp_alloc */
    PyType_GenericNew,              /* tp_new */
};

//
// module definition
//

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "sonic",
  0,                    // m_doc
  -1,                   // m_size
  /*
  sonic_methods,        // m_methods
  NULL,                 // m_reload
  NULL,                 // m_traverse
  NULL,                 // m_clear
  NULL                  // m_free
  */
};

PyObject *PyInit_sonic(void) {
    extern PyTypeObject py_sonic_t;
    if ( PyType_Ready( &py_sonic_t ) < 0)
        return NULL;

    PyObject *module = PyModule_Create( &moduledef );

    Py_INCREF( &py_sonic_t );
    if ( PyModule_AddObject(module, "Sonic", (PyObject *)&py_sonic_t) < 0) {
        Py_DECREF(&py_sonic_t);
        Py_DECREF(module);
        return NULL;
    }

    PyModule_AddObject(module, "__version__", PyUnicode_FromString(VERSION));

    if (PyErr_Occurred())
        PyErr_SetString(PyExc_ImportError, "sonic: init failed");

    return module;
}
