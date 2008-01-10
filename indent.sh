#!/bin/sh

indent \
-nbad -bap -nbc -bbo -br -bls -ncdb -ce -cp1 -cs -di2 -cli0 \
-ndj -nfc1 -nfca -hnl -i4 -ip5 -lp -pcs -nprs -psl -saf -sai \
-saw -nsc -nsob -l100 \
\
-T st_oop_t \
-T st_smi_t \
-T st_association_t \
-T st_array_t \
-T st_byte_array_t \
-T st_float_array_t \
-T st_header_t \
-T st_heap_object_t \
-T st_large_integer_t \
-T st_behavior_t \
-T st_class_t \
-T st_metaclass_t \
-T st_compiled_code_t \
-T st_compiled_method_t \
-T st_compiled_block_t \
-T st_lexer_t \
-T st_lexer_token_t \
-T st_lexer_token_type_t \
-T st_lexer_error_t \
-T st_lexer_error_code_t \
\
$@
