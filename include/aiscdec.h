#ifndef AISCDEC_H_INCLUDED
#define AISCDEC_H_INCLUDED

#include <json-c/json.h>

#ifdef __cplusplus
extern "C" {
#endif
    
// -- Class body
    
typedef struct _aiscdec_t aiscdec_t;
    
// -- ctr/dtr

// Initialises the Python runtime and threading system if necessary
aiscdec_t *
aiscdec_new ();

void
aiscdec_destroy (aiscdec_t **self_p);

// -- Decode an AIS message

json_object *
aiscdec_decode (aiscdec_t *self, const char *body, size_t padding);

#ifdef __cplusplus
}
#endif

#endif  // AISCDEC_H_INCLUDED
