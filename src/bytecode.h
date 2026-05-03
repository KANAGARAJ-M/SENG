
#ifndef SENG_BYTECODE_H
#define SENG_BYTECODE_H

#include <stdint.h>

/* ── .sec file format ────────────────────────────────────────
   Header  :  "SENG" (4 bytes)  +  version uint8_t
   Consts  :  uint32_t count, then each:
                 uint8_t type  (0=number, 1=string)
                 double  or  uint32_t+chars
   Code    :  uint32_t instruction count
              each: uint8_t opcode, int32_t operand
──────────────────────────────────────────────────────────── */

#define SEC_MAGIC   "SENG"
#define SEC_VERSION  1

typedef enum {
    OP_PUSH_NUM,    /* push constants[op]              */
    OP_PUSH_STR,    /* push constants[op] as string    */
    OP_PUSH_TRUE,
    OP_PUSH_FALSE,
    OP_PUSH_NULL,
    OP_LOAD,        /* push env[constants[op]]         */
    OP_STORE,       /* env[constants[op]] = pop()      */
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_NEG,
    OP_NOT,
    OP_CMP_EQ,
    OP_CMP_NEQ,
    OP_CMP_GT,
    OP_CMP_LT,
    OP_CMP_GTE,
    OP_CMP_LTE,
    OP_AND,
    OP_OR,
    OP_PRINT,
    OP_INPUT,       /* op = name-index: prompt top, read, store */
    OP_JUMP,        /* unconditional jump to op                 */
    OP_JUMP_FALSE,  /* pop; if !truthy jump to op               */
    OP_CALL,        /* op = arg count                           */
    OP_RET,
    OP_HALT,
    OP_LIST_NEW,    /* new list, store in constants[op]         */
    OP_LIST_PUSH,   /* pop val; push to list constants[op]      */
    OP_LIST_GET,    /* pop idx (1-based); push item             */
    OP_LIST_LEN,    /* push length of list constants[op]        */
    OP_POP,
    OP_DEF_FUNC,    /* op=name-idx, next = param_count, then param name idxes */
    OP_CLASS_DEF,   /* op=name-idx, next=field-count, then next=method-count, then methods */
    OP_NEW_INST,    /* op=class-name-idx, next=instance-name-idx, next=arg-count */
    OP_GET_PROP,    /* op=prop-name-idx, obj on stack                    */
    OP_SET_PROP,    /* op=prop-name-idx, obj and val on stack            */
    OP_ME,          /* push 'me' instance                                */
    OP_METHOD_CALL, /* op=method-name-idx, next=arg-count, obj on stack  */
    OP_TRY,         /* op=catch-jump-offset                              */
    OP_THROW,       /* pop error value and jump to current catch         */
    OP_END_TRY,     /* pop catch frame                                   */
    OP_IMPORT,      /* op=package-name-index                             */
    OP_COUNT
} Opcode;

#endif /* SENG_BYTECODE_H */
