# aiscdec
Pure C interface to the libais AIS decoder


Rationale
---------

Kurt Schwehr's [libais](https://github.com/schwehr/libais/) is the de facto
standard open source AIS decoder, with by far the largest and most active
community.

It solves two main problems which are relevant here:

1. Decode the AIS wire format to an easy-to-use set of in-memory types
2. Convert all of these types into a single representation, for easy consumption
   by library clients.

(1) is implemented as a portable C++ library, while (2) takes the form of a
Python library written in C++, outputting a PyDict object for each decoded
AIS message.

For performance and ease of integration it's often highly desirable to use
(2) directly from native code, so this library wraps the Python interface
behind an easy to use C api, indirecting through a linked Python runtime
as necessary. It's a lot simpler (and faster) in practice than it sounds.

To prevent project clients needing to work with raw Python data types (and thus
grappling with the Global Interpreter Lock) we convert the PyDict objects
into json-c objects, which are able to carry all the necessary semantics.
json-c seems sufficiently widely used to assume that all interested clients
will be able to access it easily.

Note that you don't have to install the libais Python package to use this
project, as we bundle all the C++ code through a git submodule. The only
build and runtime dependencies are json-c and python2.7 (including dev
headers).


Example
-------

```C:
#include <aiscdec.h>
...

// Decoder objects set up the python runtime if necessary, and are reusable
aiscdec_t *dec = aiscdec_new ();

// So call _decode() on them as many times as you want
json_object *obj = aiscdec_decode (dec, "177KQJ5000G?tO`K>RA1wUbN0TKH", 0);

// Access returned data through the standard json-c api
json_object *js_mmsi = NULL;
json_object_object_get_ex (obj, "mmsi", &js_mmsi);
int mmsi = json_object_get_int (js_mmsi);
assert (mmsi == 477553000);

// And convert to strings in the same way
puts (json_object_to_json_string_ext (obj, JSON_C_TO_STRING_PRETTY));

// Remember to delete the decoder object when you're done
aiscdec_destroy (&dec);
```


Full API
--------

```C
// -- ctr/dtr

// Initialises the Python runtime and threading system if necessary
aiscdec_t *
aiscdec_new ();

void
aiscdec_destroy (aiscdec_t **self_p);
    
// -- Decode an AIS message

json_object *
aiscdec_decode (aiscdec_t *self, const char *body, size_t padding);
```

Building
--------

First install python2.7 and json-c from your package repositories; under
Ubuntu they're called `python2.7-dev` and `libjson-c-dev`.

Then build with CMake in the usual way; here we run the tests as well:

```
mkdir build && cd build
cmake ..
make
./run_tests
```

Make sure to use the --recursive flag when you git clone this project, as we
include libais as a submodule.


Caveats
-------

I haven't yet tested this code as thouroughly as I normally do, so it's probably
going to be a bit sketchy right now - expect a few strange bugs until I come back.
There are plenty of asserts sprinkled around, though, so as long as you don't
set NDEBUG it should fail loudly if there's a problem. I'll also add some more
extensive docs in due course.
