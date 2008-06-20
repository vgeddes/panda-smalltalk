
#ifndef __ST_UNICODE_H__
#define __ST_UNICODE_H__

#include <st-types.h>
#include <unistd.h>

void       st_unicode_init (void);

void       st_unicode_canonical_decomposition (const st_unichar *in,
					       int inlen,
					       st_unichar **out,
					       int *outlen);

#define st_utf8_skip(c) (((0xE5000000 >> (((c) >> 3) & 0xFE)) & 3) + 1)
#define st_utf8_next_char(p) (char *)((p) + st_utf8_skip (*(const char *)(p)))

int         st_utf8_strlen            (const char *string);
st_unichar  st_utf8_get_unichar       (const char *p);
bool        st_utf8_validate          (const char *string, ssize_t max_len);
int         st_unichar_to_utf8        (st_unichar ch, char *outbuf);
const char *st_utf8_offset_to_pointer (const char *string, st_uint offset);
st_unichar *st_utf8_to_ucs4           (const char *string);

#endif /* __ST_UNICODE_H__ */
