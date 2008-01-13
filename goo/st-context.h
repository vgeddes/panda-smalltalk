



typedef struct
{
    st_header_t header;

    st_oop_t parent;
    st_oop_t method;
    st_oop_t stack;
    st_oop_t sp;
    st_oop_t ip;
    
    st_oop_t ap;
    st_oop_t tp;
        
} st_context_t;

/* Context BlockContext MethodContext */
