#include <roa/datadrive.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/* Config and Internal helpers
 */

#ifndef ROA_DD_MAX_RULE
#define ROA_DD_MAX_RULE 128
#endif

/* Limit of uint64_t for sugar reasons */
#define ROA_DD_MAX_DATA 64

/* This is our fallback clz */
#define ROA_BIT_IDX(n) \
        (n == (1UL << 0) ? 0 : \
         n == (1UL << 1) ? 1 : \
         n == (1UL << 2) ? 2 : \
         n == (1UL << 3) ? 3 : \
         n == (1UL << 4) ? 4 : \
         n == (1UL << 5) ? 5 : \
         n == (1UL << 6) ? 6 : \
         n == (1UL << 7) ? 7 : \
         n == (1UL << 8) ? 8 : \
         n == (1UL << 9) ? 9 : \
         n == (1UL << 10) ? 10 : \
         n == (1UL << 11) ? 11 : \
         n == (1UL << 12) ? 12 : \
         n == (1UL << 13) ? 13 : \
         n == (1UL << 14) ? 14 : \
         n == (1UL << 15) ? 15 : \
         n == (1UL << 16) ? 16 : \
         n == (1UL << 17) ? 17 : \
         n == (1UL << 18) ? 18 : \
         n == (1UL << 19) ? 19 : \
         n == (1UL << 20) ? 20 : \
         n == (1UL << 21) ? 21 : \
         n == (1UL << 22) ? 22 : \
         n == (1UL << 23) ? 23 : \
         n == (1UL << 24) ? 24 : \
         n == (1UL << 25) ? 25 : \
         n == (1UL << 26) ? 26 : \
         n == (1UL << 27) ? 27 : \
         n == (1UL << 28) ? 28 : \
         n == (1UL << 29) ? 29 : \
         n == (1UL << 30) ? 30 : \
         n == (1UL << 31) ? 31 : \
         n == (1UL << 32) ? 32 : \
         n == (1UL << 33) ? 33 : \
         n == (1UL << 34) ? 34 : \
         n == (1UL << 35) ? 35 : \
         n == (1UL << 36) ? 36 : \
         n == (1UL << 37) ? 37 : \
         n == (1UL << 38) ? 38 : \
         n == (1UL << 39) ? 39 : \
         n == (1UL << 40) ? 40 : \
         n == (1UL << 41) ? 41 : \
         n == (1UL << 42) ? 42 : \
         n == (1UL << 43) ? 43 : \
         n == (1UL << 44) ? 44 : \
         n == (1UL << 45) ? 45 : \
         n == (1UL << 46) ? 46 : \
         n == (1UL << 47) ? 47 : \
         n == (1UL << 48) ? 48 : \
         n == (1UL << 49) ? 49 : \
         n == (1UL << 50) ? 50 : \
         n == (1UL << 51) ? 51 : \
         n == (1UL << 52) ? 52 : \
         n == (1UL << 53) ? 53 : \
         n == (1UL << 54) ? 54 : \
         n == (1UL << 55) ? 55 : \
         n == (1UL << 56) ? 56 : \
         n == (1UL << 57) ? 57 : \
         n == (1UL << 58) ? 58 : \
         n == (1UL << 59) ? 59 : \
         n == (1UL << 60) ? 60 : \
         n == (1UL << 61) ? 61 : \
         n == (1UL << 62) ? 62 : \
         n == (1UL << 63) ? 63 : 0)

/* We convert id's to array indexs.
 */
static int
clz(uint64_t id) {
        #if defined(__GNUC__) || defined(__clang__)
        return __builtin_clzl(id);
        #else
        /* This isn't a CLZ impl, but for our purposes it works
         * since we assert the number is also a pow2
         */
        return ROA_BIT_IDX(id);
        #endif
        /* Missin MSVC
         * Need to test _BitScanReverse and or __lzcnt
         */
}

/* Doesn't handle the zero case */
#define ROA_IS_POW2(n) ((n & (n - 1UL)) == 0UL)

/* -------------------------------------------------------------------------- */
/* Internal Context 
 * Our context is an opaque type.
 */

struct roa_datadrive_row {
        int count;
        uintptr_t data[ROA_DD_MAX_DATA_COUNT];
};

struct roa_datadrive_rule_pair {
        uint64_t on;  /* bit flags that must be on */
        uint64_t off; /* bit flags that must be off */
};

struct roa_datadrive_rule {
        int count;
        int current_rule;

        struct roa_datadrive_rule_pair settings[ROA_DD_MAX_RULE];
        roa_datadrive_func funcs[ROA_DD_MAX_RULE]; 
};

struct roa_datadrive_ctx {
        uint64_t state;

        struct roa_datadrive_row rows[ROA_DD_MAX_DATA];
        struct roa_datadrive_rule rules;
};

/* -------------------------------------------------------------------------- */
/* Lifetime
 * The create functions don't do much, but its a good place for some runtime
 * checks to happen so we encorage it to happen.
 */

struct roa_datadrive_ctx*
roa_datadrive_create() {

        void *space = malloc(roa_datadrive_size_needed());

        return roa_datadrive_create_ex(
               &space);
}

struct roa_datadrive_ctx*
roa_datadrive_create_ex(
        void **space)
{
        /* Do a runtime check of ROA_BIT_IDX, since this is a hand rolled
         * list of things, we'll do a check each time we create a context.
         *
         * Also do a runtime check for ROA_IS_POW2.
         *
         * Also assert for compatability.
         */

        for(int i = 0; i < 64; ++i) {
                assert(ROA_BIT_IDX((1UL << i)) == i  && "ROA_BIT_IDX is corrupt");
                assert(ROA_IS_POW2((1UL << i)) && "Pow2 Macro is faulty");
        }

        memset(*space, 0, roa_datadrive_size_needed());
        return *space;
}

size_t
roa_datadrive_size_needed()
{
        return sizeof(struct roa_datadrive_ctx);
}

void
roa_datadrive_destroy(
        struct roa_datadrive_ctx **ctx)
{
        assert(ctx && "'ctx' must not be null");

        free(*ctx);
        *ctx = 0;
}

/* -------------------------------------------------------------------------- */
/* Rules 
 */

void
roa_datadrive_rule_add(
        struct roa_datadrive_ctx *ctx,
        roa_datadrive_func func,
        uint64_t on,
        uint64_t off)
{
        /* Check the state of the inputs
         */

        assert(ctx && "'ctx' must not be null");
        assert(func && "'func' need a callback function");
        assert(ctx->rules.count < ROA_DD_MAX_RULE && "Maxed out of rules");

        int idx = ctx->rules.count;

        ctx->rules.settings[idx].on = on;
        ctx->rules.settings[idx].off = off;
        ctx->rules.funcs[idx] = func;

        ctx->rules.count += 1;
}

static void
roa_datadrive_exec_not_found(
        struct roa_datadrive_ctx *ctx)
{
        (void)ctx;
        /* This function is only used when we can't find a function to run */
}

int
roa_datadrive_rule_execute(
        struct roa_datadrive_ctx *ctx)
{
        /* Check the state of the inputs
         */

        assert(ctx && "'ctx' must not be null");

        int curr_idx = ctx->rules.current_rule;
        int max_idx = ctx->rules.count;
        struct roa_datadrive_rule_pair *pair = ctx->rules.settings;
        
        roa_datadrive_func f = roa_datadrive_exec_not_found;
        uint64_t st = ctx->state;

        for(int i = 0; i < ROA_DD_MAX_DATA; ++i) {
                int idx = curr_idx + i;
                idx = idx % max_idx;

                /* The way this works is this
                 * on | off - this sets all the bit flags for this rule.
                 * st & (on|off) this should equal only the on bits
                 * if it doesn't then we have data present that invalidates
                 * this rule from executing.
                 */
                
                uint64_t res = st & (pair[idx].on | pair[idx].off);

                if(res == pair[idx].on) {
                        f = ctx->rules.funcs[idx];
                        ctx->rules.current_rule = idx;
                        break;
                }
        }

        f(ctx);

        return (f != roa_datadrive_exec_not_found);
}

/* -------------------------------------------------------------------------- */
/* Data 
*/

int
roa_datadrive_data_set(
        struct roa_datadrive_ctx *ctx,
        uint64_t data_id,
        uintptr_t data)
{
        /* Check the state of the inputs
         */

        assert(ctx && "'ctx' must not be null");
        assert(data_id && "'data_id'  must be greater than zero");
        assert(ROA_IS_POW2(data_id) && "'data_id' must be a power of two");

        /* Insert index
         * Try and find a space for this data. We can store multiple instances
         * of the same type type upto ROA_DD_MAX_DATA_COUNT.
         */

        int idx = clz(data_id);
        struct roa_datadrive_row *row = &ctx->rows[idx];
        int insert_idx = row->count;

        if(insert_idx >= ROA_DD_MAX_DATA_COUNT) {
                assert(!"Failed to add data, buffer out of space");
                return 0;
        }

        /* If we have space add the data and also update the data state.
         */

        row->data[insert_idx] = data;
        row->count += 1;
        ctx->state |= data_id;

        return 1;
}

void
roa_datadrive_data_clear(
        struct roa_datadrive_ctx *ctx,
        uint64_t data_id)
{
        /* Check the state of the inputs
         */

        assert(ctx && "'ctx' must not be null");
        assert(data_id && "'data_id'  must be greater than zero");
        assert(ROA_IS_POW2(data_id) && "'data_id' must be a power of two");

        /* Clear the bit flag,
         * clear the data.
         * then clear the counter.
         */

        ctx->state &= ~(data_id);

        int idx = clz(data_id);
        struct roa_datadrive_row *row = &ctx->rows[idx];

        /* Clear out the data
         */

        memset(row->data, 0, sizeof(row->data));

        row->count = 0;
}

void
roa_datadrive_data_get(
        struct roa_datadrive_ctx *ctx,
        uint64_t data_id,
        uintptr_t *out_data,
        int *out_count)
{
        /* Check the state of the inputs
         */

        assert(ctx && "'ctx' must not be null");
        assert(data_id && "'data_id'  must be greater than zero");
        assert(ROA_IS_POW2(data_id) && "'data_id' must be a power of two");
        assert((out_data || out_count) && "'out_data' or 'out_count' must be set");

        /* Get the Row for this data.
         */

        int idx = clz(data_id);
        struct roa_datadrive_row *row = &ctx->rows[idx];

        /* If the user wants the size update the size
         */

        if(out_count) {
                *out_count = row->count;
        }

        /* If the user wants the data copy the data accross
         */

        if(out_data) {
                size_t bytes = sizeof(row->data[0]) * row->count;
                memcpy(out_data, row->data, bytes);
        }
}

/* -------------------------------------------------------------------------- */
/* Config 
 */

#undef ROA_DD_MAX_RULE
#undef ROA_DD_MAX_DATA 
#undef ROA_BIT_IDX

