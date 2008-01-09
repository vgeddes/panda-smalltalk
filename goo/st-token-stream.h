#ifndef __ST_TOKEN_STREAM_H__
#define __ST_TOKEN_STREAM_H__

#include <glib.h>
#include <st-lexer.h>

G_BEGIN_DECLS

typedef struct _st_token_stream st_token_stream_t

st_token_stream_t  *st_token_stream_new              (st_lexer_t *lexer);

st_token_type_t     st_token_stream_look_ahead       (st_token_stream_t *stream, int i);

st_lexer_token_t   *st_token_stream_look_ahead_token (st_token_stream_t *stream, int k);

void             st_token_stream_consume          (st_token_stream_t *stream);

st_lexer_t        *st_token_stream_get_lexer        (st_token_stream_t *stream);

guint            st_token_stream_get_index        (st_token_stream_t *stream);

guint            st_token_stream_mark             (st_token_stream_t *stream);

void             st_token_stream_rewind           (st_token_stream_t *stream);

void             st_token_stream_rewind_to_index  (st_token_stream_t *stream);

void             st_token_stream_add_filter       (st_token_stream_t *stream,
						    st_token_type_t    token_type);

G_END_DECLS

#endif /* __ST_TOKEN_STREAM_H__ */
