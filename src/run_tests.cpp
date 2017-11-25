#include "aiscdec.h"

#include <cstdio>
#include <cassert>
#include <cstring>

#include <json-c/json.h>

#ifdef NDEBUG
#undef NDEBUG
#endif


//  --------------------------------------------------------------------------
//  Utils

static bool streq (const char *s1, const char *s2) {
    return strcmp (s1, s2) == 0;
}


//  --------------------------------------------------------------------------
//  JSON contents reading

static int
obj_get_int_val (json_object *obj, const char *key)
{
    json_object *val = NULL;
    bool found = json_object_object_get_ex (obj, key, &val);
    assert (found);

    assert (json_object_is_type (val, json_type_int));
    return json_object_get_int (val);
}

static const char *
obj_get_str_val (json_object *obj, const char *key)
{
    json_object *val = NULL;
    bool found = json_object_object_get_ex (obj, key, &val);
    assert (found);
    
    assert (json_object_is_type (val, json_type_string));
    return json_object_get_string (val);
}


//  --------------------------------------------------------------------------
//  main() - the tests happen here
    
int main () {
    aiscdec_t *dec = aiscdec_new ();
    assert (dec);

    json_object *obj = NULL;

    // -- Type 1 message
    
    obj = aiscdec_decode (dec, "177KQJ5000G?tO`K>RA1wUbN0TKH", 0);
    assert (obj);

    assert (477553000 == obj_get_int_val (obj, "mmsi"));

    printf ("## OUTPUT: %s\n",
            json_object_to_json_string_ext (obj, JSON_C_TO_STRING_PRETTY));

    // TODO more tests of its contents

    // -- Type 5 message

    obj = aiscdec_decode
            (dec,
             "55P5TL01VIaAL@7WKO@mBplU@<PDhh000000001S;AJ::4A80?4i@E53"
             "1@0000000000000",
             2);
    assert (obj);

    assert (streq (obj_get_str_val (obj, "destination"),
                   "SEATTLE@@@@@@@@@@@@@"));

    printf ("## OUTPUT: %s\n",
            json_object_to_json_string_ext (obj, JSON_C_TO_STRING_PRETTY));

    // TODO more tests of its contents
    
    // TODO more message types

    aiscdec_destroy (&dec);

    puts ("");
    puts ("## All tests passed");
}
