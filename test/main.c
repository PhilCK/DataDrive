#include <assert.h>
#include <roa/datadrive.h>
#include <stdio.h>

enum {
        LINE_DATA = ROA_BIT(1),
        MESH_DATA = ROA_BIT(2),
        PLAYER_SETUP = ROA_BIT(3),
        GAME_STATE = ROA_BIT(4),
        LOADING_DATA = ROA_BIT(5),
        RESOURCE_DATA = ROA_BIT(6),
        DATA_SYSTEMS = ROA_BIT(7),
        DATA_SHUTDOWN = ROA_BIT(8),
};

void
startup(struct roa_datadrive_ctx *ctx)
{
        printf("Startup\n");
        roa_datadrive_data_set(ctx, DATA_SYSTEMS, 0);
};

void
loading(struct roa_datadrive_ctx *ctx)
{
        printf("loading\n");
        roa_datadrive_data_set(ctx, RESOURCE_DATA, 0);
        roa_datadrive_data_set(ctx, PLAYER_SETUP, 0);
}

void
player_select(struct roa_datadrive_ctx *ctx)
{
        printf("Player select\n");

        static int count = 0;
        count += 1;

        if(count >= 4) {
                roa_datadrive_data_set(ctx, GAME_STATE, 4);
                roa_datadrive_data_set(ctx, GAME_STATE, 5);
                roa_datadrive_data_set(ctx, GAME_STATE, 6);
        }
}

void
game_play_update(struct roa_datadrive_ctx *ctx)
{
        printf("Game update\n");

        static int count = 0;
        count += 1;

        uintptr_t data[ROA_DD_MAX_DATA_COUNT];
        roa_datadrive_data_get(ctx, GAME_STATE, data, NULL);

        assert(data[0] == 4);
        assert(data[1] == 5);
        assert(data[2] == 6);

        if(count >= 8) {
               roa_datadrive_data_clear(ctx, PLAYER_SETUP);
        };
}

void
game_over_update(struct roa_datadrive_ctx *ctx)
{
        printf("Game Over update\n");

        roa_datadrive_data_clear(ctx, GAME_STATE);
        roa_datadrive_data_set(ctx, DATA_SHUTDOWN, 0);
}

void
shutdown(struct roa_datadrive_ctx *ctx)
{
        printf("shutdown\n");
        roa_datadrive_data_clear(ctx, DATA_SHUTDOWN);
}

int
main() {
        struct roa_datadrive_ctx *dd_ctx = roa_datadrive_create();

        printf("LINE_DATA: %d\n", LINE_DATA);
        printf("MESH_DATA: %d\n", MESH_DATA);
        printf("PLAYER_SETUP: %d\n", PLAYER_SETUP);
        printf("GAME_STATE: %d\n", GAME_STATE);
        printf("LOADING_DATA: %d\n", LOADING_DATA);
        printf("RESOURCE_DATA: %d\n", RESOURCE_DATA);
        printf("DATA_SYSTEMS: %d\n", DATA_SYSTEMS);

        roa_datadrive_rule_add(
                dd_ctx,
                startup,
                0,
                DATA_SYSTEMS|RESOURCE_DATA);

        roa_datadrive_rule_add(
                dd_ctx,
                loading,
                DATA_SYSTEMS,
                RESOURCE_DATA);

        roa_datadrive_rule_add(
                dd_ctx,
                player_select,
                PLAYER_SETUP,
                GAME_STATE);

        roa_datadrive_rule_add(
                dd_ctx,
                game_play_update,
                PLAYER_SETUP|GAME_STATE,
                0);

        roa_datadrive_rule_add(
                dd_ctx,
                game_over_update,
                GAME_STATE,
                PLAYER_SETUP);

        roa_datadrive_rule_add(
                dd_ctx,
                shutdown,
                DATA_SHUTDOWN,
                0);

        while(roa_datadrive_rule_execute(dd_ctx)) {

        }

        roa_datadrive_destroy(&dd_ctx);

        return 0;
}

