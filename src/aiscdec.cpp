#include "aiscdec.h"

#include <cassert>

#include <Python.h>


struct _aiscdec_t {
    PyObject *pymod_libais;
    PyObject *pyfun_libais_decode;
    // holder for the args we call python ais.decode() with
    PyObject *pytup_decode_callargs;  
};


//  --------------------------------------------------------------------------
//  libais ais_py.cpp functions

namespace libais {
    extern "C"
    void init_ais ();
}


//  --------------------------------------------------------------------------
//  ctr/dtr

extern "C"    
aiscdec_t *
aiscdec_new ()
{
    aiscdec_t *self = (aiscdec_t *) calloc (1, sizeof(*self));
    assert (self);

    Py_Initialize ();
    PyEval_InitThreads();

    // -- BEGIN PYTHON GIL SECTION
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    // -- Load the libais module

    libais::init_ais ();

    self->pymod_libais = PyImport_ImportModule ("_ais");
    assert (self->pymod_libais);

    // -- Load the libais decode function

    self->pyfun_libais_decode = PyObject_GetAttrString (self->pymod_libais,
                                                        "decode");
    assert (self->pyfun_libais_decode);

    // -- Make the args holder for calls to ais.decode()

    self->pytup_decode_callargs = PyTuple_New (2);
    assert (self->pytup_decode_callargs);

    PyGILState_Release(gstate);
    // -- END PYTHON GIL SECTION
    
    return self;
}

extern "C"
void
aiscdec_destroy (aiscdec_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        aiscdec_t *self = *self_p;
        
        Py_DECREF (self->pyfun_libais_decode);
        Py_DECREF (self->pymod_libais);
        Py_DECREF (self->pytup_decode_callargs);

        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Decoding an AIS message - internal python driver
//    NB you MUST take the GIL before calling this

static PyObject *
s_py_decode_ais (aiscdec_t *self, const char *body, size_t padding)
{
    assert (self);
    assert (body);

    PyObject *pystr_body = PyString_FromString (body);
    assert (pystr_body);

    PyObject *pyint_padding = PyInt_FromLong (padding);
    assert (pyint_padding);

    // NB tuple takes ownership of new members
    int rc;
    rc = PyTuple_SetItem (self->pytup_decode_callargs, 0, pystr_body);
    assert (!rc);
    rc = PyTuple_SetItem (self->pytup_decode_callargs, 1, pyint_padding);
    assert (!rc);

    // TODO handle errors better
    PyObject *res = PyObject_CallObject (self->pyfun_libais_decode,
                                         self->pytup_decode_callargs);

    return res;
}


//  --------------------------------------------------------------------------
//  Converting pydicts to json objs
//    NB you MUST take the GIL before calling this
//    
//    We only go one level deep into the object, and only copy across
//    values that are strings, ints, longs or floats (python types)

static json_object *
s_pydict_to_json (PyObject *dict)
{
    assert (dict);
    assert (PyDict_Check (dict));  // check is indeed a dict

    json_object *res = json_object_new_object ();
    assert (res);

    // Copy the dict vals into the json
    { 
        PyObject *key, *value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(dict, &pos, &key, &value)) {
            assert (PyUnicode_Check (key));
            // Decref'd at the end of the while loop, as key_str points inside
            PyObject *key_str_obj = PyUnicode_AsUTF8String (key);
            assert (key_str_obj);
            
            const char *key_str = PyString_AsString (key_str_obj);

            // Switch on the type of the value the key points to
            if (PyString_Check (value)) {
                const char *val_str = PyString_AsString (value);
                
                json_object *jsval = json_object_new_string (val_str);
                assert (jsval);
                json_object_object_add (res, key_str, jsval);
                
            } else
            if (PyUnicode_Check (value)) {
                PyObject *val_str_obj = PyUnicode_AsUTF8String (value);
                assert (val_str_obj);
                const char *val_str = PyString_AsString (val_str_obj);

                json_object *jsval = json_object_new_string (val_str);
                assert (jsval);
                json_object_object_add (res, key_str, jsval);

                Py_DECREF (val_str_obj);
                
            } else
            if (PyInt_Check (value)) {
                long val_long = PyInt_AsLong (value);
                int val_int = (int) val_long;  // fine unless bug in libais

                json_object *jsval = json_object_new_int (val_int);
                assert (jsval);
                json_object_object_add (res, key_str, jsval);

            } else
            if (PyLong_Check (value)) {
                long val_long = PyLong_AsLong (value);
                int val_int = (int) val_long;  // fine unless bug in libais

                json_object *jsval = json_object_new_int (val_int);
                assert (jsval);
                json_object_object_add (res, key_str, jsval);
                
            } else
            if (PyFloat_Check (value)) {
                double val_dub = PyFloat_AsDouble (value);

                json_object *jsval = json_object_new_double (val_dub);
                assert (jsval);
                json_object_object_add (res, key_str, jsval);
                
            }
            else {
                // TODO don't assert, pass, but this is useful for dev
                assert (0 && "Unknown type");
            }

            Py_DECREF (key_str_obj);
        }  // while(){}
    }

    return res;
}


//  --------------------------------------------------------------------------
//  Class interface for decoding message

extern "C"
json_object *
aiscdec_decode (aiscdec_t *self, const char *body, size_t padding)
{
    assert (self);
    assert (body);

    // -- Python interaction section (inc making json)
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    
    PyObject *pyobj = s_py_decode_ais (self, body, padding);
    assert (pyobj);

    json_object *res = s_pydict_to_json (pyobj);

    PyGILState_Release(gstate);
    // -- Python section ends

    return res;
}

