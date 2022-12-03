#include <nfa_builder.h>
#include <fcntl.h>
#include <unistd.h>
#include <compiler_errors.h>

#ifdef _DEBUG
#include <assert.h>
#endif

static int nfa_simple(nfa_t*, char);
static int nfa_concat(nfa_t* restrict, nfa_t* restrict);
static int nfa_union(nfa_t* restrict, nfa_t* restrict);
static int nfa_star(nfa_t*);
static int nfa_init(nfa_t*, size_t);
static void nfa_deinit(nfa_t*);
static int nfa_state_init(state_t*, bool);
static void nfa_state_deinit(state_t*);
static int nfa_state_addsymbol(state_t*, char, int);
static int nfa_state_extend(state_t*);
static int nfa_delta(nfa_t*, char);

/*** EXPORTED ***/

int nfa_build(nfa_t* nfa_left, const node_t* parse_tree){
    nfa_t nfa_right;

    #ifdef _DEBUG
    assert(parse_tree != NULL);
    #endif
    
    switch(parse_tree->op){

        case NONE:
            ERROR_RETHROW(nfa_simple(nfa_left, parse_tree->c));
            break;

        case CONCAT:

            //check on the integrity of the parse tree
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL && parse_tree->r_child != NULL);
            #endif

            ERROR_RETHROW(nfa_build(nfa_left, parse_tree->l_child));
            ERROR_RETHROW(nfa_build(&nfa_right, parse_tree->r_child), nfa_destroy(nfa_left)); 
            ERROR_RETHROW(nfa_concat(nfa_left, &nfa_right), nfa_destroy(nfa_left); nfa_destroy(&nfa_right));

    
            break;

        case UNION:
            
            //check on the integrity of the parse tree
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL && parse_tree->r_child != NULL);
            #endif

            ERROR_RETHROW(nfa_build(nfa_left, parse_tree->l_child));
            ERROR_RETHROW(nfa_build(&nfa_right, parse_tree->r_child), nfa_destroy(nfa_left));
            ERROR_RETHROW(nfa_union(nfa_left, &nfa_right), nfa_destroy(nfa_left); nfa_destroy(&nfa_right));
            
            break;

        case STAR:
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL);
            #endif

            ERROR_RETHROW(nfa_build(nfa_left, parse_tree->l_child));
            ERROR_RETHROW(nfa_star(nfa_left), nfa_destroy(nfa_left));
            
        default: break;
    }

    return OK;
}

int nfa_accepts(nfa_t* nfa, const char* string, bool* result){
    *result = false;

    size_t i;
    if ((nfa->current_states = malloc(sizeof(int) * nfa->states_len)) == NULL)
    {
        return BAD_ALLOCATION;
    }

    nfa->current_states_capacity = nfa->states_len;

    // INITIAL STATE
    nfa->current_states[0] = 0;
    nfa->current_states_len = 1;
    
    for (i=0; string[i] != '\0'; ++i)
    {
        ERROR_RETHROW(
            nfa_delta(nfa, string[i]),
            free(nfa->current_states)
        );  
    }

    for (i=0; i < nfa->current_states_len; ++i)
    {

        int idx = nfa->current_states[i];
        if (nfa->states[idx].final)
        {
            *result = true;
        }
    }

    free(nfa->current_states);
    return OK;
}


void nfa_destroy(nfa_t* nfa){

    if (nfa == NULL)
    {
        return;
    }

    size_t i;
    for (i=0; i<nfa->states_len; ++i)
    {
        nfa_state_deinit(&nfa->states[i]);
    }
    
    nfa_deinit(nfa);
}

/* DEBUGGING */

int nfa_graph(const nfa_t* nfa){
	FILE* f;
	if ( (f = fopen("nfa_graph.gv", "w")) == NULL)
		return -1;
	
	fputs("digraph G{", f);
	size_t i;
    for (i=0; i<nfa->states_len; ++i){
        const char* bgcolor = (nfa->states[i].final) ? "red" : "white";
        fprintf(f, "%lu [label=\"%lu\", style=\"filled\", fillcolor=\"%s\", shape=\"oval\"]\n", i, i, bgcolor);
        
        size_t j;
        for (j=0; j<nfa->states[i].len; ++j){
            fprintf(f, "%lu -> %d [label=\"%c\"]\n", i, nfa->states[i].mapped_state[j], nfa->states[i].charset[j]);
        }
    }
	fputs("}", f);
	
	fclose(f);
    return 0;
}

void nfa_compare(const nfa_t* restrict nfa1, const nfa_t* restrict nfa2)
{
    if (nfa1->states_len != nfa2->states_len)
    {
        fprintf(stderr, "different: %s @ line %d\n", __FILE__, __LINE__);
        return;
    }

    size_t i;
    for (i=0; i<nfa1->states_len; ++i)
    {
        if (nfa1->states[i].len != nfa2->states[i].len)
        {
            fprintf(stderr, "different: %s @ line %d @ iteration %lu\n", __FILE__, __LINE__, i);
            return;
        }

        size_t j;
        for (j=0; j<nfa1->states[i].len; ++j)
        {
            if (nfa1->states[i].charset[j] != nfa2->states[i].charset[j])
            {
                fprintf(stderr, "different: %s @ line %d @ iteration %lu\n", __FILE__, __LINE__, j);
                return;
            }

            if (nfa1->states[i].mapped_state[j] != nfa2->states[i].mapped_state[j])
            {
                fprintf(stderr, "different: %s @ line %d @ iteration %lu\n", __FILE__, __LINE__, j);
                return;
            }
        }
    }

    fprintf(stderr, "NFA1 == NFA2\n");

}

// Serialize Object and save
int nfa_collection_save(const nfa_t* nfa, size_t count, const char* filename)
{
    #ifdef _DEBUG
    assert(nfa != NULL);
    assert(filename != NULL);
    assert(count > 0);
    #endif

    // Open file
    int fd;
    if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0)
    {
        return IO_ERROR;
    }

    // try to write the length of the collection
    if (write(fd, &count, sizeof(size_t)) < (ssize_t) sizeof(size_t))
    {
        close(fd);
        return IO_ERROR;
    }

    // for every nfa in the collection
    size_t i;
    for (i=0; i<count; ++i)
    {
        // try to write the states count for nfa i
        if (write(fd, &(nfa[i].states_len), sizeof(size_t)) < (ssize_t) sizeof(size_t))
        {
            close(fd);
            return IO_ERROR;
        }

        // for every state in nfa i
        size_t j;
        for (j=0; j<nfa[i].states_len; ++j)
        {
            // try to write the length of the charset for that state
            if (write(fd, &(nfa[i].states[j].len), sizeof(size_t)) < (ssize_t) sizeof(size_t))
            {
                close(fd);
                return IO_ERROR;
            }

            if (nfa[i].states[j].len > 0)
            {

                // try to write the charset
                if (write(fd, nfa[i].states[j].charset, sizeof(char) * nfa[i].states[j].len) < (ssize_t) (sizeof(char) * nfa[i].states[j].len))
                {
                    close(fd);
                    return IO_ERROR;
                }

                // try to write the mapped states
                if (write(fd, nfa[i].states[j].mapped_state, sizeof(int) * nfa[i].states[j].len) < (ssize_t) (sizeof(int) * nfa[i].states[j].len))
                {
                    close(fd);
                    return IO_ERROR;
                }
            }

            // try to write the finality of the state
            if (write(fd, &(nfa[i].states[j].final), sizeof(bool)) < (ssize_t) sizeof(bool))
            {
                close(fd);
                return IO_ERROR;
            }
        }
    }
    

    close(fd);
    return OK;
}

void nfa_collection_delete(nfa_t* nfa_list, size_t len)
{
    #ifdef _DEBUG
    assert(nfa_list != NULL);
    #endif

    size_t i;                                           
    for (i=0; i<len; ++i)                             
    {                                                   
        size_t j;
        for (j=0; j<nfa_list[i].states_len; ++j)
        {
            nfa_state_deinit(&(nfa_list[i].states[j]));
        }
    
        free(nfa_list[i].states);                         
    }                   
                                    
    free(nfa_list);                                   
}

int nfa_collection_load(nfa_t** nfa_collection, size_t* len, const char* filename)
{
    #ifdef _DEBUG
    assert(nfa_collection != NULL);
    assert(filename != NULL);
    #endif

    // try to open file in reading mode
    int fd;
    if ((fd = open(filename, O_RDONLY)) < 0)
    {
        return IO_ERROR;
    }

    // try to read 8 bytes for the length of the collection
    size_t count = 0;
    if (read(fd, &count, sizeof(size_t)) < (ssize_t) sizeof(size_t))
    {
        close(fd);
        return IO_ERROR;
    }

    // if count is 0, some ting wong
    if (count == 0)
    {
        close(fd);
        return INVALID_FORMAT;
    }

    // try to allocate a new nfa collection to a temporary pointer
    nfa_t* temp;
    if ((temp = calloc(sizeof(nfa_t), count)) == NULL)
    {
        close(fd);
        return BAD_ALLOCATION;
    }

    size_t i;
    for (i=0; i<count; ++i)
    {
        if (read(fd, &(temp[i].states_len), sizeof(size_t)) < (ssize_t) sizeof(size_t))
        {
            close(fd);
            nfa_collection_delete(temp, count);
            return IO_ERROR;
        }

        if (temp[i].states_len == 0)
        {
            close(fd);
            nfa_collection_delete(temp, count);
            return INVALID_FORMAT;
        }
        
        if ((temp[i].states = calloc(sizeof(state_t), temp[i].states_len)) == NULL)
        {
            close(fd);
            nfa_collection_delete(temp, count);
            return BAD_ALLOCATION;
        }

        size_t j;
        for (j=0; j<temp[i].states_len; ++j)
        {

            if (read(fd, &(temp[i].states[j].len), sizeof(size_t)) < (ssize_t) sizeof(size_t))
            {
                close(fd);
                nfa_collection_delete(temp, count);
                return IO_ERROR;
            }

            if (temp[i].states[j].len > 0)
            {
                if ((temp[i].states[j].charset = calloc(sizeof(char), (temp[i].states[j].len))) == NULL)
                {
                    close(fd);
                    nfa_collection_delete(temp, count);
                    return BAD_ALLOCATION;
                }

                if ((temp[i].states[j].mapped_state = calloc(sizeof(int), (temp[i].states[j].len))) == NULL)
                {
                    close(fd);
                    nfa_collection_delete(temp, count);
                    return BAD_ALLOCATION;
                }

                if (read(fd, temp[i].states[j].charset, sizeof(char) * (temp[i].states[j].len)) < (ssize_t) (sizeof(char) * (temp[i].states[j].len)))
                {
                    close(fd);
                    nfa_collection_delete(temp, count);
                    return IO_ERROR;
                }

                if (read(fd, temp[i].states[j].mapped_state, sizeof(int) * (temp[i].states[j].len)) < (ssize_t) (sizeof(int) * (temp[i].states[j].len)))
                {
                    close(fd);
                    nfa_collection_delete(temp, count);
                    return IO_ERROR;
                }

            }

            if (read(fd, &(temp[i].states[j].final), sizeof(bool)) < (ssize_t) sizeof(bool))
            {
                close(fd);
                nfa_collection_delete(temp, count);
                return IO_ERROR;
            }

            temp[i].states[j].capacity = temp[i].states[j].len;
        }

        temp->current_states = NULL;
        temp->current_states_len = 0;
        temp->current_states_capacity = 0;
    }

    *nfa_collection = temp;
    *len = count;
    
    close(fd);
    return OK;
}

/*** INTERNAL ***/


static int nfa_delta(nfa_t* nfa, char c){
    int* next_states;
    if ((next_states = malloc(sizeof(int) * nfa->current_states_len)) == NULL)
        return BAD_ALLOCATION;

    size_t next_states_len = 0;
    size_t next_states_capacity = nfa->current_states_len;

    size_t i;
    for (i=0; i<nfa->current_states_len; ++i){
        int k = nfa->current_states[i];

        size_t j;        
        for (j=0; j<nfa->states[k].len; ++j){

            if (nfa->states[k].charset[j] == c)
            {
                
                if (next_states_len >= next_states_capacity)
                {
                    if ((next_states = reallocarray(next_states, next_states_capacity * 2, sizeof(int))) == NULL)
                    {
                        free(next_states);
                        return BAD_ALLOCATION;
                    }
                    next_states_capacity *= 2;
                }

                next_states[next_states_len++] = nfa->states[k].mapped_state[j];
            }
        }
    }

    free(nfa->current_states);
    nfa->current_states = next_states;
    nfa->current_states_len = next_states_len;
    nfa->current_states_capacity = next_states_capacity;

    return 0;

}

static int nfa_init(nfa_t* nfa, size_t n_states){
    nfa_t tmp_nfa;
    
    if ((tmp_nfa.states = calloc((size_t) n_states, sizeof(state_t))) == NULL)
    {
        return BAD_ALLOCATION;
    }

    tmp_nfa.states_len = n_states;

    *nfa = tmp_nfa;
    return 0;
}

static int nfa_simple(nfa_t* nfa, char c){
    nfa_t temp;
    ERROR_RETHROW(nfa_init(&temp, 3));
    
    ERROR_RETHROW(
        nfa_state_init(&temp.states[0], false),
        nfa_destroy(&temp);
    );
    
    ERROR_RETHROW(
        nfa_state_init(&temp.states[1], true),
        nfa_destroy(&temp);
    );
    
    ERROR_RETHROW(
        nfa_state_addsymbol(&temp.states[0], c, 1),
        nfa_destroy(&temp);
    );

    *nfa = temp;
    return 0;
}

static int nfa_concat(nfa_t* restrict nfa_left, nfa_t* restrict nfa_right){
    nfa_t *nfa1 = nfa_left;
    nfa_t *nfa2 = nfa_right;

    // new nfa
    nfa_t nfa;

    ERROR_RETHROW(
        nfa_init(&nfa, nfa1->states_len + nfa2->states_len - 1)
    );

    // MOVE nfa1 AS IS
    size_t i;
    for (i=0; i<nfa1->states_len; ++i){
        nfa.states[i] = nfa1->states[i];
    }

    // MOVE nfa2 LESS INITIAL STATE AND CHANGING STATES' NUMBERS
    size_t j;
    i = nfa.states_len - 1;
    for (j=nfa2->states_len - 1; j>0; --j){
        nfa.states[i] = nfa2->states[j];
        
        size_t l;
        for (l=0; l<nfa.states[i].len; ++l){
            nfa.states[i].mapped_state[l] += nfa1->states_len - 1;
        }
        
        --i;
    }

    // CONNECT FINAL STATES OF nfa1 TO nfa2
    for (i=0; i<nfa1->states_len; ++i){
        
        if ( nfa.states[i].final ){
            
            for (j=0; j<nfa2->states[0].len; ++j)
                ERROR_RETHROW(
                        nfa_state_addsymbol(
                                &nfa.states[i], nfa2->states[0].charset[j],
                                nfa2->states[0].mapped_state[j] + nfa1->states_len - 1
                                ),
                        nfa_destroy(&nfa)
                );
            
            nfa.states[i].final = nfa2->states[0].final; //state is final only if the initial state of nfa2 was
        }
    }

    // CLEANING UP
    nfa_state_deinit(&nfa2->states[0]);
    nfa_deinit(nfa1);
    nfa_deinit(nfa2);

    *nfa_left = nfa;
    return 0;
}

static int nfa_union(nfa_t* restrict nfa_left, nfa_t* restrict nfa_right){
    nfa_t* nfa1 = nfa_left;
    nfa_t* nfa2 = nfa_right;

    nfa_t nfa;
    ERROR_RETHROW(nfa_init(&nfa, nfa1->states_len + nfa2->states_len - 1));

    // MOVE nfa1 AS IS
    size_t i;
    for (i=0; i<nfa1->states_len; ++i)
    {
        nfa.states[i] = nfa1->states[i];
    }

    // MOVE NFA2 LESS INITIAL STATE
    size_t j;
    i = nfa.states_len - 1;
    for (j=nfa2->states_len - 1; j>0; --j)
    {
        nfa.states[i] = nfa2->states[j];
        
        // REDENOMINATE NFA2 STATES' NUMBERS
        size_t l;
        for (l=0; l<nfa.states[i].len; ++l)
        {
            nfa.states[i].mapped_state[l] += nfa1->states_len-1;
        }

        --i;
    }

    // INITIAL STATE OF NFA1 IS INITIAL STATE OF NFA2: COPY NFA2 INITIAL STATE TRANSITIONS INTO THE NEW INITIAL STATE
    for (i=0; i<nfa2->states[0].len; ++i){
        ERROR_RETHROW(
            nfa_state_addsymbol(
                &nfa.states[0], 
                nfa2->states[0].charset[i], 
                nfa2->states[0].mapped_state[i] + nfa1->states_len - 1
            ),
            nfa_destroy(&nfa)
        );
    }        

    // CLEANING UP
    nfa_state_deinit(&nfa2->states[0]);
    nfa_deinit(nfa1);
    nfa_deinit(nfa2);

    *nfa_left = nfa;
    
    return 0;
}

static int nfa_star(nfa_t* nfa){
    //COPY THE INITIAL STATE TRANSITIONS INTO EVERY FINAL STATE
    size_t i;
    for (i=1; i<nfa->states_len; ++i){
        if (nfa->states[i].final){
            size_t j;
            for (j=0; j<nfa->states[0].len; ++j)
            {
                ERROR_RETHROW(
                    nfa_state_addsymbol(
                        &nfa->states[i], 
                        nfa->states[0].charset[j], 
                        nfa->states[0].mapped_state[j]
                    )
                );
            }
        }
    }
    
    nfa->states[0].final = true; //Accept null strings
    return 0;
}

static void nfa_deinit(nfa_t* nfa)
{
    if (nfa != NULL)
    {
        if (nfa->states_len > 0 && nfa->states != NULL)
        {
            free(nfa->states);
        }

        nfa->states_len = 0;
        nfa->states = NULL;
    }
}

static int nfa_state_init(state_t* state, bool final){
    state->len = 0;
    state->capacity = 0;
    state->final = false;
    
    if ((state->charset = malloc(sizeof(char) * 5)) == NULL)
    {
        return BAD_ALLOCATION;
    }

    if ((state->mapped_state = malloc(sizeof(int) * 5)) == NULL)
    {
        free(state->charset);
        return BAD_ALLOCATION;
    }
    
    state->capacity = 5;
    state->final = final;
    return OK;
}

static int nfa_state_extend(state_t* state){
    size_t new_capacity = state->capacity * 2;
    char* new_charset;
    int* new_mapped_state;

    if ((new_charset = reallocarray(state->charset, new_capacity, sizeof(char))) == NULL)
    {
        return BAD_ALLOCATION;
    }

    if ((new_mapped_state = reallocarray(state->mapped_state, new_capacity, sizeof(int))) == NULL)
    {
        free(new_charset);
        return BAD_ALLOCATION;
    }

    state->capacity = new_capacity;
    state->charset = new_charset;
    state->mapped_state = new_mapped_state;
    return 0;
}

static int nfa_state_addsymbol(state_t* state, char c, int ns){
    if (state->len >= state->capacity)
    {
        ERROR_RETHROW(nfa_state_extend(state));
    }

    state->charset[state->len] = c;
    state->mapped_state[state->len] = ns;
    ++state->len;

    return 0;
}

static void nfa_state_deinit(state_t* state){
    #ifdef _DEBUG
    assert(state != NULL);
    #endif

    if (state->capacity > 0)
    {
        if (state->charset != NULL)
        {
            free(state->charset);
        }

        if (state->mapped_state != NULL)
        {
            free(state->mapped_state);
        }
    }

    state->charset = NULL;
    state->mapped_state = NULL;

    state->capacity = 0;
    state->len = 0;
    state->final = false;
}


