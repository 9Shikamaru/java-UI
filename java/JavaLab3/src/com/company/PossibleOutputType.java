package com.company;

enum PossibleOutputType{
    STRING(String.class),
    BYTE_ARRAY(byte[].class),
    CHAR_ARRAY(char[].class);

    private final Class<?> type;

    private PossibleOutputType(Class<?> type){
        this.type = type;
    }

    public Class<?> getType(){
        return this.type;
    }
}
