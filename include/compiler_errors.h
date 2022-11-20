#ifndef _COMPILER_ERRORS_H
#define _COMPILER_ERRORS_H

enum error_codes{
    OK,
    BAD_ALLOCATION,
    ILLFORMED_REGEXPR,
    NFA_CORRUPT_TREE,
    NFA_LOAD_ERROR,
    NFA_SAVE_ERROR,
    INVALID_BUFFER
};


// error handling macro. Checks the return value for __FUNC call and if there is an error
// executes the second argument before returning the error
#define ERROR_RETHROW(__FUNC, __COMMAND...) ({ \
    int __err_code;\
    if ((__err_code = __FUNC) != OK) { \
        {__COMMAND __VA_OPT__(;)} \
        return __err_code; \
    }\
})



#endif