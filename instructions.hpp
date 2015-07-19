#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

namespace jjde {

#define JJDE_VARIABLE_ARGUMENT_COUNT 10000
#define UNDEFINED(_1, _2) X(_1, _2)

#define JJDE_OPERATIONS_ENUM \
    X( NOP             , 0 ), \
    X( ACONST_NULL     , 0 ), \
    X( ICONST_M1       , 0 ), \
    X( ICONST_0        , 0 ), \
    X( ICONST_1        , 0 ), \
    X( ICONST_2        , 0 ), \
    X( ICONST_3        , 0 ), \
    X( ICONST_4        , 0 ), \
    X( ICONST_5        , 0 ), \
    X( LCONST_0        , 0 ), \
    X( LCONST_1        , 0 ), \
    X( FCONST_0        , 0 ), \
    X( FCONST_1        , 0 ), \
    X( FCONST_2        , 0 ), \
    X( DCONST_0        , 0 ), \
    X( DCONST_1        , 0 ), \
    X( BIPUSH          , 1 ), \
    X( SIPUSH          , 2 ), \
    X( LDC             , 1 ), \
    X( LDC_W           , 2 ), \
    X( LDC2_W          , 2 ), \
    X( ILOAD           , 1 ), \
    X( LLOAD           , 1 ), \
    X( FLOAD           , 1 ), \
    X( DLOAD           , 1 ), \
    X( ALOAD           , 1 ), \
    X( ILOAD_0         , 0 ), \
    X( ILOAD_1         , 0 ), \
    X( ILOAD_2         , 0 ), \
    X( ILOAD_3         , 0 ), \
    X( LLOAD_0         , 0 ), \
    X( LLOAD_1         , 0 ), \
    X( LLOAD_2         , 0 ), \
    X( LLOAD_3         , 0 ), \
    X( FLOAD_0         , 0 ), \
    X( FLOAD_1         , 0 ), \
    X( FLOAD_2         , 0 ), \
    X( FLOAD_3         , 0 ), \
    X( DLOAD_0         , 0 ), \
    X( DLOAD_1         , 0 ), \
    X( DLOAD_2         , 0 ), \
    X( DLOAD_3         , 0 ), \
    X( ALOAD_0         , 0 ), \
    X( ALOAD_1         , 0 ), \
    X( ALOAD_2         , 0 ), \
    X( ALOAD_3         , 0 ), \
    X( IALOAD          , 0 ), \
    X( LALOAD          , 0 ), \
    X( FALOAD          , 0 ), \
    X( DALOAD          , 0 ), \
    X( AALOAD          , 0 ), \
    X( BALOAD          , 0 ), \
    X( CALOAD          , 0 ), \
    X( SALOAD          , 0 ), \
    X( ISTORE          , 1 ), \
    X( LSTORE          , 1 ), \
    X( FSTORE          , 1 ), \
    X( DSTORE          , 1 ), \
    X( ASTORE          , 1 ), \
    X( ISTORE_0        , 0 ), \
    X( ISTORE_1        , 0 ), \
    X( ISTORE_2        , 0 ), \
    X( ISTORE_3        , 0 ), \
    X( LSTORE_0        , 0 ), \
    X( LSTORE_1        , 0 ), \
    X( LSTORE_2        , 0 ), \
    X( LSTORE_3        , 0 ), \
    X( FSTORE_0        , 0 ), \
    X( FSTORE_1        , 0 ), \
    X( FSTORE_2        , 0 ), \
    X( FSTORE_3        , 0 ), \
    X( DSTORE_0        , 0 ), \
    X( DSTORE_1        , 0 ), \
    X( DSTORE_2        , 0 ), \
    X( DSTORE_3        , 0 ), \
    X( ASTORE_0        , 0 ), \
    X( ASTORE_1        , 0 ), \
    X( ASTORE_2        , 0 ), \
    X( ASTORE_3        , 0 ), \
    X( IASTORE         , 0 ), \
    X( LASTORE         , 0 ), \
    X( FASTORE         , 0 ), \
    X( DASTORE         , 0 ), \
    X( AASTORE         , 0 ), \
    X( BASTORE         , 0 ), \
    X( CASTORE         , 0 ), \
    X( SASTORE         , 0 ), \
    X( POP             , 0 ), \
    X( POP2            , 0 ), \
    X( DUP             , 0 ), \
    X( DUP_X1          , 0 ), \
    X( DUP_X2          , 0 ), \
    X( DUP2            , 0 ), \
    X( DUP2_X1         , 0 ), \
    X( DUP2_X2         , 0 ), \
    X( SWAP            , 0 ), \
    X( IADD            , 0 ), \
    X( LADD            , 0 ), \
    X( FADD            , 0 ), \
    X( DADD            , 0 ), \
    X( ISUB            , 0 ), \
    X( LSUB            , 0 ), \
    X( FSUB            , 0 ), \
    X( DSUB            , 0 ), \
    X( IMUL            , 0 ), \
    X( LMUL            , 0 ), \
    X( FMUL            , 0 ), \
    X( DMUL            , 0 ), \
    X( IDIV            , 0 ), \
    X( LDIV            , 0 ), \
    X( FDIV            , 0 ), \
    X( DDIV            , 0 ), \
    X( IREM            , 0 ), \
    X( LREM            , 0 ), \
    X( FREM            , 0 ), \
    X( DREM            , 0 ), \
    X( ISHL            , 0 ), \
    X( LSHL            , 0 ), \
    X( ISHR            , 0 ), \
    X( LSHR            , 0 ), \
    X( IUSHR           , 0 ), \
    X( LUSHR           , 0 ), \
    X( IAND            , 0 ), \
    X( LAND            , 0 ), \
    X( IOR             , 0 ), \
    X( LOR             , 0 ), \
    X( IXOR            , 0 ), \
    X( LXOR            , 0 ), \
    X( IINC            , 2 ), \
    X( I2L             , 0 ), \
    X( I2F             , 0 ), \
    X( I2D             , 0 ), \
    X( L2I             , 0 ), \
    X( L2F             , 0 ), \
    X( L2D             , 0 ), \
    X( F2I             , 0 ), \
    X( F2L             , 0 ), \
    X( F2D             , 0 ), \
    X( D2I             , 0 ), \
    X( D2L             , 0 ), \
    X( D2F             , 0 ), \
    X( I2B             , 0 ), \
    X( I2C             , 0 ), \
    X( I2S             , 0 ), \
    X( LCMP            , 0 ), \
    X( FCMPL           , 0 ), \
    X( FCMPG           , 0 ), \
    X( DCMPL           , 0 ), \
    X( DCMPG           , 0 ), \
    X( IFEQ            , 2 ), \
    X( IFNE            , 2 ), \
    X( IFLT            , 2 ), \
    X( IFGE            , 2 ), \
    X( IFGT            , 2 ), \
    X( IFLE            , 2 ), \
    X( IF_ICMPEQ       , 2 ), \
    X( IF_ICMPNE       , 2 ), \
    X( IF_ICMPLT       , 2 ), \
    X( IF_ICMPGE       , 2 ), \
    X( IF_ICMPGT       , 2 ), \
    X( IF_ICMPLE       , 2 ), \
    X( IF_ACMPEQ       , 2 ), \
    X( IF_ACMPNE       , 2 ), \
    X( GOTO            , 2 ), \
    X( JSR             , 2 ), \
    X( RET             , 1 ), \
    X( TABLESWITCH     , JJDE_VARIABLE_ARGUMENT_COUNT ), \
    X( LOOKUPSWITCH    , JJDE_VARIABLE_ARGUMENT_COUNT ), \
    X( IRETURN         , 0 ), \
    X( LRETURN         , 0 ), \
    X( FRETURN         , 0 ), \
    X( DRETURN         , 0 ), \
    X( ARETURN         , 0 ), \
    X( RETURN          , 0 ), \
    X( GETSTATIC       , 2 ), \
    X( PUTSTATIC       , 2 ), \
    X( GETFIELD        , 2 ), \
    X( PUTFIELD        , 2 ), \
    X( INVOKEVIRTUAL   , 2 ), \
    X( INVOKESPECIAL   , 2 ), \
    X( INVOKESTATIC    , 2 ), \
    X( INVOKEINTERFACE , 4 ), \
    X( INVOKEDYMANIC   , 4 ), \
    X( NEW             , 2 ), \
    X( NEWARRAY        , 1 ), \
    X( ANEWARRAY       , 2 ), \
    X( ARRAYLENGTH     , 0 ), \
    X( ATHROW          , 0 ), \
    X( CHECKCAST       , 2 ), \
    X( INSTANCEOF      , 2 ), \
    X( MONITORENTER    , 0 ), \
    X( MONITOREXIT     , 0 ), \
    X( WIDE            , JJDE_VARIABLE_ARGUMENT_COUNT ), \
    X( MULTIANEWARRAY  , 3 ), \
    X( IFNULL          , 2 ), \
    X( IFNONNULL       , 2 ), \
    X( GOTO_W          , 4 ), \
    X( JSR_W           , 4 ), \
    X( BREAKPOINT      , 0 ), \
    UNDEFINED( cb , 0 ), \
    UNDEFINED( cc , 0 ), \
    UNDEFINED( cd , 0 ), \
    UNDEFINED( ce , 0 ), \
    UNDEFINED( cf , 0 ), \
    UNDEFINED( d0 , 0 ), \
    UNDEFINED( d1 , 0 ), \
    UNDEFINED( d2 , 0 ), \
    UNDEFINED( d3 , 0 ), \
    UNDEFINED( d4 , 0 ), \
    UNDEFINED( d5 , 0 ), \
    UNDEFINED( d6 , 0 ), \
    UNDEFINED( d7 , 0 ), \
    UNDEFINED( d8 , 0 ), \
    UNDEFINED( d9 , 0 ), \
    UNDEFINED( da , 0 ), \
    UNDEFINED( db , 0 ), \
    UNDEFINED( dc , 0 ), \
    UNDEFINED( dd , 0 ), \
    UNDEFINED( de , 0 ), \
    UNDEFINED( df , 0 ), \
    UNDEFINED( e0 , 0 ), \
    UNDEFINED( e1 , 0 ), \
    UNDEFINED( e2 , 0 ), \
    UNDEFINED( e3 , 0 ), \
    UNDEFINED( e4 , 0 ), \
    UNDEFINED( e5 , 0 ), \
    UNDEFINED( e6 , 0 ), \
    UNDEFINED( e7 , 0 ), \
    UNDEFINED( e8 , 0 ), \
    UNDEFINED( e9 , 0 ), \
    UNDEFINED( ea , 0 ), \
    UNDEFINED( eb , 0 ), \
    UNDEFINED( ec , 0 ), \
    UNDEFINED( ed , 0 ), \
    UNDEFINED( ee , 0 ), \
    UNDEFINED( ef , 0 ), \
    UNDEFINED( f0 , 0 ), \
    UNDEFINED( f1 , 0 ), \
    UNDEFINED( f2 , 0 ), \
    UNDEFINED( f3 , 0 ), \
    UNDEFINED( f4 , 0 ), \
    UNDEFINED( f5 , 0 ), \
    UNDEFINED( f6 , 0 ), \
    UNDEFINED( f7 , 0 ), \
    UNDEFINED( f8 , 0 ), \
    UNDEFINED( f9 , 0 ), \
    UNDEFINED( fa , 0 ), \
    UNDEFINED( fb , 0 ), \
    UNDEFINED( fc , 0 ), \
    UNDEFINED( fd , 0 ), \
    X( IMPDEP1         , 0 ), \
    X( IMPDEP2         , 0 )


struct Instruction {
    // Actual enum values
    enum Operation {
#define X(_1, _2) _1
        JJDE_OPERATIONS_ENUM
#undef X
    };

    // Matching strings
    static const std::string name[] = {
#define X(_1, _2) #_1
        JJDE_OPERATIONS_ENUM
#undef X
    };

    // Operand counts
    static const std::size_t argument_count[] = {
#define X(_1, _2) _2
        JJDE_OPERATIONS_ENUM
#undef X
    };

    Operation operation;
    std::vector<unsigned char> arguments;
};

}

#endif // INSTRUCTIONS_HPP
