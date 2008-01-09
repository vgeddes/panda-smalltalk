
#include <st-token-stream.h>


struct _GooTokenStream
{
	st_lexer_t *lexer;

	GArray   *tokens;

	int       p;
	int       n;
	
	GList *filters;
};


static void
fill_buffer (GooTokenStream *stream)
{
	st_lexer_t *lexer = stream->lexer;
	st_lexer_token_t *token;
	
	int i=0;
	
	do {
		token = st_lexer_next_token (lexer);
	
		for (GList *l = stream->filters; l; l=l->next) {
			if (st_lexer_token_type (token) == GPOINTER_TO_INT (l->data))
				continue;
	
		g_array_append_val (stream->tokens, token);	
	
	} while (st_lexer_token_type (token) != ST_TOKEN_EOF) {

}

GooTokenStream *
st_token_stream_new (st_lexer_t *lexer)
{
	g_assert (lexer != NULL);
	
	GooTokenStream *stream = g_new0 (GooTokenStream, 1);
	
	stream->lexer = lexer;
	
	stream->tokens = g_array_sized_new (false, true,
					    sizeof (st_lexer_token_t*),
					    256);

	stream->filters = NULL;
	stream->p = 0;

	fill_buffer (stream);

	return stream;
}

st_token_type_t
st_token_stream_look_ahead (GooTokenStream *stream, int k)
{
	g_assert (stream != NULL);

	

}

st_lexer_token_t *
st_token_stream_look_ahead_token (GooTokenStream *stream, int i)
{
	g_assert (stream != NULL);

	/* undefined */
	if (i == 0) {
		return NULL;
	}

	if (i < 0) {
	
		i++;
		
		if ((stream->p + i - 1) < 0) {
			// invalid
			return g_array_index(tokens, st_lexer_token_t *, ;
		}
	
	}
	
	if ((stream->p + i - 1) >= stream->n) {
		return ST_STRING_STREAM_EOF;	
	}

	return stream->text[stream->p + i - 1];

}

void
st_token_stream_consume (GooTokenStream *stream)
{
	g_assert (stream != NULL);

	if (stream->p < stream->n) {
	
		stream->p++;
	
	}
}

st_lexer_t *
st_token_stream_get_lexer (GooTokenStream *stream)
{
	g_assert (stream != NULL);
	
	return stream->lexer;
}

void
st_token_stream_add_filter (GooTokenStream *stream,
			     st_token_type_t    token_type)
{
	stream->filters = g_list_append (stream->filters, GINT_TO_POINTER (token_type));
}

