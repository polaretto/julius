#include "market.h"

#include "building/warehouse.h"
#include "core/calc.h"
#include "game/resource.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

struct resource_data {
    int building_id;
    int distance;
    int num_buildings;
};

int building_market_get_max_food_stock(building *market)
{
    int max_stock = 0;
    if (market->id > 0 && market->type == BUILDING_MARKET) {
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
            int stock = market->data.market.inventory[i];
            if (stock > max_stock) {
                max_stock = stock;
            }
        }
    }
    return max_stock;
}

int building_market_get_max_goods_stock(building *market)
{
    int max_stock = 0;
    if (market->id > 0 && market->type == BUILDING_MARKET) {
        for (int i = INVENTORY_MIN_GOOD; i < INVENTORY_MAX_GOOD; i++) {
            int stock = market->data.market.inventory[i];
            if (stock > max_stock) {
                max_stock = stock;
            }
        }
    }
    return max_stock;
}

static void update_food_resource(struct resource_data *data, resource_type resource, const building *b, int distance)
{
    if (b->data.storage.resourceStored[resource]) {
        data->num_buildings++;
        if (distance < data->distance) {
            data->distance = distance;
            data->building_id = b->id;
        }
    }
}

static void update_good_resource(struct resource_data *data, resource_type resource, building *b, int distance)
{
    if (!Data_CityInfo.resourceStockpiled[resource] &&
        building_warehouse_get_amount(b, resource) > 0) {
        data->num_buildings++;
        if (distance < data->distance) {
            data->distance = distance;
            data->building_id = b->id;
        }
    }
}

int building_market_get_storage_destination(building *market)
{
    struct resource_data resources[INVENTORY_MAX];
    for (int i = 0; i < INVENTORY_MAX; i++) {
        resources[i].building_id = 0;
        resources[i].num_buildings = 0;
        resources[i].distance = 40;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->type != BUILDING_GRANARY && b->type != BUILDING_WAREHOUSE) {
            continue;
        }
        if (!b->hasRoadAccess || b->distanceFromEntry <= 0 ||
            b->roadNetworkId != market->roadNetworkId) {
            continue;
        }
        int distance = calc_maximum_distance(market->x, market->y, b->x, b->y);
        if (distance >= 40) {
            continue;
        }
        if (b->type == BUILDING_GRANARY) {
            if (scenario_property_rome_supplies_wheat()) {
                continue;
            }
            update_food_resource(&resources[INVENTORY_WHEAT], RESOURCE_WHEAT, b, distance);
            update_food_resource(&resources[INVENTORY_VEGETABLES], RESOURCE_VEGETABLES, b, distance);
            update_food_resource(&resources[INVENTORY_FRUIT], RESOURCE_FRUIT, b, distance);
            update_food_resource(&resources[INVENTORY_MEAT], RESOURCE_MEAT, b, distance);
        } else if (b->type == BUILDING_WAREHOUSE) {
            // goods
            update_good_resource(&resources[INVENTORY_WINE], RESOURCE_WINE, b, distance);
            update_good_resource(&resources[INVENTORY_OIL], RESOURCE_OIL, b, distance);
            update_good_resource(&resources[INVENTORY_POTTERY], RESOURCE_POTTERY, b, distance);
            update_good_resource(&resources[INVENTORY_FURNITURE], RESOURCE_FURNITURE, b, distance);
        }
    }

    // update demands
    if (market->data.market.potteryDemand) {
        market->data.market.potteryDemand--;
    } else {
        resources[INVENTORY_POTTERY].num_buildings = 0;
    }
    if (market->data.market.furnitureDemand) {
        market->data.market.furnitureDemand--;
    } else {
        resources[INVENTORY_FURNITURE].num_buildings = 0;
    }
    if (market->data.market.oilDemand) {
        market->data.market.oilDemand--;
    } else {
        resources[INVENTORY_OIL].num_buildings = 0;
    }
    if (market->data.market.wineDemand) {
        market->data.market.wineDemand--;
    } else {
        resources[INVENTORY_WINE].num_buildings = 0;
    }

    int canGo = 0;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        if (resources[i].num_buildings) {
            canGo = 1;
            break;
        }
    }
    if (!canGo) {
        return 0;
    }
    // prefer food if we don't have it
    if (!market->data.market.inventory[INVENTORY_WHEAT] && resources[INVENTORY_WHEAT].num_buildings) {
        market->data.market.fetchInventoryId = INVENTORY_WHEAT;
        return resources[INVENTORY_WHEAT].building_id;
    } else if (!market->data.market.inventory[INVENTORY_VEGETABLES] && resources[INVENTORY_VEGETABLES].num_buildings) {
        market->data.market.fetchInventoryId = INVENTORY_VEGETABLES;
        return resources[INVENTORY_VEGETABLES].building_id;
    } else if (!market->data.market.inventory[INVENTORY_FRUIT] && resources[INVENTORY_FRUIT].num_buildings) {
        market->data.market.fetchInventoryId = INVENTORY_FRUIT;
        return resources[INVENTORY_FRUIT].building_id;
    } else if (!market->data.market.inventory[INVENTORY_MEAT] && resources[INVENTORY_MEAT].num_buildings) {
        market->data.market.fetchInventoryId = INVENTORY_MEAT;
        return resources[INVENTORY_MEAT].building_id;
    }
    // then prefer resource if we don't have it
    if (!market->data.market.inventory[INVENTORY_POTTERY] && resources[INVENTORY_POTTERY].num_buildings) {
        market->data.market.fetchInventoryId = INVENTORY_POTTERY;
        return resources[INVENTORY_POTTERY].building_id;
    } else if (!market->data.market.inventory[INVENTORY_FURNITURE] && resources[INVENTORY_FURNITURE].num_buildings) {
        market->data.market.fetchInventoryId = INVENTORY_FURNITURE;
        return resources[INVENTORY_FURNITURE].building_id;
    } else if (!market->data.market.inventory[INVENTORY_OIL] && resources[INVENTORY_OIL].num_buildings) {
        market->data.market.fetchInventoryId = INVENTORY_OIL;
        return resources[INVENTORY_OIL].building_id;
    } else if (!market->data.market.inventory[INVENTORY_WINE] && resources[INVENTORY_WINE].num_buildings) {
        market->data.market.fetchInventoryId = INVENTORY_WINE;
        return resources[INVENTORY_WINE].building_id;
    }
    // then prefer smallest stock below 50
    int min_stock = 50;
    int fetch_inventory = -1;
    if (resources[INVENTORY_WHEAT].num_buildings &&
        market->data.market.inventory[INVENTORY_WHEAT] < min_stock) {
        min_stock = market->data.market.inventory[INVENTORY_WHEAT];
        fetch_inventory = INVENTORY_WHEAT;
    }
    if (resources[INVENTORY_VEGETABLES].num_buildings &&
        market->data.market.inventory[INVENTORY_VEGETABLES] < min_stock) {
        min_stock = market->data.market.inventory[INVENTORY_VEGETABLES];
        fetch_inventory = INVENTORY_VEGETABLES;
    }
    if (resources[INVENTORY_FRUIT].num_buildings &&
        market->data.market.inventory[INVENTORY_FRUIT] < min_stock) {
        min_stock = market->data.market.inventory[INVENTORY_FRUIT];
        fetch_inventory = INVENTORY_FRUIT;
    }
    if (resources[INVENTORY_MEAT].num_buildings &&
        market->data.market.inventory[INVENTORY_MEAT] < min_stock) {
        min_stock = market->data.market.inventory[INVENTORY_MEAT];
        fetch_inventory = INVENTORY_MEAT;
    }
    if (resources[INVENTORY_POTTERY].num_buildings &&
        market->data.market.inventory[INVENTORY_POTTERY] < min_stock) {
        min_stock = market->data.market.inventory[INVENTORY_POTTERY];
        fetch_inventory = INVENTORY_POTTERY;
    }
    if (resources[INVENTORY_FURNITURE].num_buildings &&
        market->data.market.inventory[INVENTORY_FURNITURE] < min_stock) {
        min_stock = market->data.market.inventory[INVENTORY_FURNITURE];
        fetch_inventory = INVENTORY_FURNITURE;
    }
    if (resources[INVENTORY_OIL].num_buildings &&
        market->data.market.inventory[INVENTORY_OIL] < min_stock) {
        min_stock = market->data.market.inventory[INVENTORY_OIL];
        fetch_inventory = INVENTORY_OIL;
    }
    if (resources[INVENTORY_WINE].num_buildings &&
        market->data.market.inventory[INVENTORY_WINE] < min_stock) {
        fetch_inventory = INVENTORY_WINE;
    }

    if (fetch_inventory == -1) {
        // all items well stocked: pick food below threshold
        if (resources[INVENTORY_WHEAT].num_buildings &&
            market->data.market.inventory[INVENTORY_WHEAT] < 600) {
            fetch_inventory = INVENTORY_WHEAT;
        }
        if (resources[INVENTORY_VEGETABLES].num_buildings &&
            market->data.market.inventory[INVENTORY_VEGETABLES] < 400) {
            fetch_inventory = INVENTORY_VEGETABLES;
        }
        if (resources[INVENTORY_FRUIT].num_buildings &&
            market->data.market.inventory[INVENTORY_FRUIT] < 400) {
            fetch_inventory = INVENTORY_FRUIT;
        }
        if (resources[INVENTORY_MEAT].num_buildings &&
            market->data.market.inventory[INVENTORY_MEAT] < 400) {
            fetch_inventory = INVENTORY_MEAT;
        }
    }
    if (fetch_inventory < 0 || fetch_inventory >= INVENTORY_MAX) {
        return 0;
    }
    market->data.market.fetchInventoryId = fetch_inventory;
    return resources[fetch_inventory].building_id;
}
