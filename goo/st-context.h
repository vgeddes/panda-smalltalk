



typedef struct
{
    STHeader header;

    st_oop parent;
    st_oop method;
    st_oop stack;
    st_oop sp;
    st_oop ip;
    
    st_oop ap;
    st_oop tp;
        
} st_context_t;

/* Context BlockContext MethodContext */
