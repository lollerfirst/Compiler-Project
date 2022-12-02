#ifndef _COMPILER_ERRORS_H
#define _COMPILER_ERRORS_H

enum error_codes{
    OK,
    BAD_ALLOCATION,
    ILLFORMED_REGEXPR,
    NFA_CORRUPT_TREE,
    NFA_LOAD_ERROR,
    NFA_SAVE_ERROR,
    INVALID_BUFFER,
    IO_ERROR,
    NOT_SERIALIZABLE,
    INVALID_FORMAT,
    INVALID_TOKEN,
};


// error handling macro. Checks the return value for __FUNC call and if there is an error
// executes the second argument before returning the error
#define ERROR_RETHROW(__FUNC, ...)              \
do{                                             \
    int __err_code;                             \
    if ((__err_code = (__FUNC)) != OK) {        \
        do{__VA_ARGS__;}while(0);               \
        return __err_code;                      \
    }                                           \
}while(0)



#endif