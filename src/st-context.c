
#include "st-context.h"
#include "st-array.h"
#include "st-method.h"
#include "st-object.h"
#include "st-behavior.h"
#include "st-universe.h"

st_oop
st_message_new (st_oop selector, st_oop arguments)
{
    st_oop object;

    object = st_object_new (st_global_get ("Message"));

    ST_OBJECT_FIELDS (object)[0] = selector;
    ST_OBJECT_FIELDS (object)[1] = arguments;

    return object;
}
