#ifndef DATA_DRIVE_7E7F0EAA_26D4_405A_8586_4080F850B519
#define DATA_DRIVE_7E7F0EAA_26D4_405A_8586_4080F850B519

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* Types and Constants
 */

struct roa_datadrive_ctx;
typedef void (*roa_datadrive_func)(struct roa_datadrive_ctx*);

#define ROA_DD_MAX_DATA_COUNT 32
#define ROA_BIT(n) (1UL << n)

/* -------------------------------------------------------------------------- */
/* Lifetime
 */

struct roa_datadrive_ctx*
roa_datadrive_create();

struct roa_datadrive_ctx*
roa_datadrive_create_ex(
        void **space);

size_t
roa_datadrive_size_needed();

void
roa_datadrive_destroy(
        struct roa_datadrive_ctx **ctx);

/* -------------------------------------------------------------------------- */
/* Rule 
 */

void
roa_datadrive_rule_add(
        struct roa_datadrive_ctx *ctx,
        roa_datadrive_func func, 
        uint64_t on,
        uint64_t off);

/* Finds the next rule to execute
 */
int
roa_datadrive_rule_execute(
        struct roa_datadrive_ctx *ctx);

/* -------------------------------------------------------------------------- */
/* Data
 */

int
roa_datadrive_data_set(
        struct roa_datadrive_ctx *ctx,
        uint64_t data_id,
        uintptr_t data);

void
roa_datadrive_data_get(
        struct roa_datadrive_ctx *ctx,
        uint64_t data_id,
        uintptr_t *out_data,
        int *out_count);

void
roa_datadrive_data_clear(
        struct roa_datadrive_ctx *ctx,
        uint64_t data_id);

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
} /* extern */
#endif

#endif

