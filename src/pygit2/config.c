#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pygit2/error.h>
#include <pygit2/types.h>
#include <pygit2/utils.h>
#include <pygit2/config.h>

extern PyTypeObject ConfigType;

int
Config_init(Config *self, PyObject *args, PyObject *kwds)
{
    char *path;
    int err;

    if (kwds) {
        PyErr_SetString(PyExc_TypeError,
                        "Repository takes no keyword arguments");
        return -1;
    }

    if (PySequence_Length(args) > 0) {
        if (!PyArg_ParseTuple(args, "s", &path)) {
            return -1;
        }
        err = git_config_open_ondisk(&self->config, path);
        if (err < 0) {
            Error_set_str(err, path);
            return -1;
        }
    } else {
        err = git_config_new(&self->config);
        if (err < 0) {
            Error_set(err);
            return -1;
        }
    }
    return 0;
}

void
Config_dealloc(Config *self)
{
    PyObject_GC_UnTrack(self);
    Py_XDECREF(self->repo);
    git_config_free(self->config);
    PyObject_GC_Del(self);
}

int
Config_traverse(Config *self, visitproc visit, void *arg)
{
    Py_VISIT(self->repo);
    return 0;
}

PyObject *
Config_open(char *c_path) {
    PyObject *py_path = Py_BuildValue("(s)", c_path);
    Config *config = PyObject_GC_New(Config, &ConfigType);

    Config_init(config, py_path, NULL);

    Py_INCREF(config);

    return (PyObject *)config;
}

PyObject *
Config_get_global_config(void)
{
    char path[GIT_PATH_MAX];
    int err;

    err = git_config_find_global(path, GIT_PATH_MAX);
    if (err < 0) {
        if (err == GIT_ENOTFOUND) {
            PyErr_SetString(PyExc_IOError, "Global config file not found.");
            return NULL;
        }
        return Error_set(err);
    }

    return Config_open(path);
}

PyObject *
Config_get_system_config(void)
{
    char path[GIT_PATH_MAX];
    int err;

    err = git_config_find_system(path, GIT_PATH_MAX);
    if (err < 0) {
        if (err == GIT_ENOTFOUND) {
            PyErr_SetString(PyExc_IOError, "System config file not found.");
            return NULL;
        }
        return Error_set(err);
    }

    return Config_open(path);
}

PyMethodDef Config_methods[] = {
    {"get_system_config", (PyCFunction)Config_get_system_config,
     METH_NOARGS | METH_STATIC,
     "Return an object representing the system configuration file."},
    {"get_global_config", (PyCFunction)Config_get_global_config,
     METH_NOARGS | METH_STATIC,
     "Return an object representing the global configuration file."},
    {NULL}
};

PyTypeObject ConfigType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_pygit2.Config",                          /* tp_name           */
    sizeof(Config),                            /* tp_basicsize      */
    0,                                         /* tp_itemsize       */
    (destructor)Config_dealloc,                /* tp_dealloc        */
    0,                                         /* tp_print          */
    0,                                         /* tp_getattr        */
    0,                                         /* tp_setattr        */
    0,                                         /* tp_compare        */
    0,                                         /* tp_repr           */
    0,                                         /* tp_as_number      */
    0,                                         /* tp_as_sequence    */
    0,                                         /* tp_as_mapping     */
    0,                                         /* tp_hash           */
    0,                                         /* tp_call           */
    0,                                         /* tp_str            */
    0,                                         /* tp_getattro       */
    0,                                         /* tp_setattro       */
    0,                                         /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_HAVE_GC,                        /* tp_flags          */
    "Configuration management",                /* tp_doc            */
    (traverseproc)Config_traverse,             /* tp_traverse       */
    0,                                         /* tp_clear          */
    0,                                         /* tp_richcompare    */
    0,                                         /* tp_weaklistoffset */
    0,                                         /* tp_iter           */
    0,                                         /* tp_iternext       */
    Config_methods,                            /* tp_methods        */
    0,                                         /* tp_members        */
    0,                                         /* tp_getset         */
    0,                                         /* tp_base           */
    0,                                         /* tp_dict           */
    0,                                         /* tp_descr_get      */
    0,                                         /* tp_descr_set      */
    0,                                         /* tp_dictoffset     */
    (initproc)Config_init,                     /* tp_init           */
    0,                                         /* tp_alloc          */
    0,                                         /* tp_new            */
};
