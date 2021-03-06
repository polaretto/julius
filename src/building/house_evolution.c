#include "house_evolution.h"

#include "building/house.h"
#include "building/model.h"
#include "core/calc.h"
#include "game/resource.h"
#include "game/time.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/tiles.h"

#include "Data/CityInfo.h"

typedef enum {
    EVOLVE = 1,
    NONE = 0,
    DEVOLVE = -1
} evolve_status;

static void reset_service_required_counters()
{
    Data_CityInfo.housesRequiringFountainToEvolve = 0;
    Data_CityInfo.housesRequiringWellToEvolve = 0;
    Data_CityInfo.housesRequiringEntertainmentToEvolve = 0;
    Data_CityInfo.housesRequiringMoreEntertainmentToEvolve = 0;
    Data_CityInfo.housesRequiringEducationToEvolve = 0;
    Data_CityInfo.housesRequiringMoreEducationToEvolve = 0;
    Data_CityInfo.housesRequiringReligionToEvolve = 0;
    Data_CityInfo.housesRequiringMoreReligionToEvolve = 0;
    Data_CityInfo.housesRequiringEvenMoreReligionToEvolve = 0;
    Data_CityInfo.housesRequiringBarberToEvolve = 0;
    Data_CityInfo.housesRequiringBathhouseToEvolve = 0;
    Data_CityInfo.housesRequiringClinicToEvolve = 0;
    Data_CityInfo.housesRequiringHospitalToEvolve = 0;
    Data_CityInfo.housesRequiringFoodToEvolve = 0;

    Data_CityInfo.housesRequiringSchool = 0;
    Data_CityInfo.housesRequiringLibrary = 0;
    Data_CityInfo.housesRequiringBarber = 0;
    Data_CityInfo.housesRequiringBathhouse = 0;
    Data_CityInfo.housesRequiringClinic = 0;
    Data_CityInfo.housesRequiringReligion = 0;
}

static int check_evolve_desirability(building *house)
{
    int level = house->subtype.houseLevel;
    const model_house *model = model_get_house(level);
    int evolve_des = model->evolve_desirability;
    if (level >= HOUSE_LUXURY_PALACE) {
        evolve_des = 1000;
    }
    int current_des = house->desirability;
    int status;
    if (current_des <= model->devolve_desirability) {
        status = DEVOLVE;
    } else if (current_des >= evolve_des) {
        status = EVOLVE;
    } else {
        status = NONE;
    }
    house->data.house.evolveTextId = status; // BUG? -1 in an unsigned char?
    return status;
}

static int has_required_goods_and_services(building *house, int for_upgrade)
{
    int level = house->subtype.houseLevel;
    if (for_upgrade) {
        ++level;
    }
    const model_house *model = model_get_house(level);
    // water
    int water = model->water;
    if (!house->hasWaterAccess) {
        if (water >= 2) {
            ++Data_CityInfo.housesRequiringFountainToEvolve;
            return 0;
        }
        if (water == 1 && !house->hasWellAccess) {
            ++Data_CityInfo.housesRequiringWellToEvolve;
            return 0;
        }
    }
    // entertainment
    int entertainment = model->entertainment;
    if (house->data.house.entertainment < entertainment) {
        if (house->data.house.entertainment) {
            ++Data_CityInfo.housesRequiringMoreEntertainmentToEvolve;
        } else {
            ++Data_CityInfo.housesRequiringEntertainmentToEvolve;
        }
        return 0;
    }
    // education
    int education = model->education;
    if (house->data.house.education < education) {
        if (house->data.house.education) {
            ++Data_CityInfo.housesRequiringMoreEducationToEvolve;
        } else {
            ++Data_CityInfo.housesRequiringEducationToEvolve;
        }
        return 0;
    }
    if (education == 2) {
        ++Data_CityInfo.housesRequiringSchool;
        ++Data_CityInfo.housesRequiringLibrary;
    } else if (education == 1) {
        ++Data_CityInfo.housesRequiringSchool;
    }
    // religion
    int religion = model->religion;
    if (house->data.house.numGods < religion) {
        if (religion == 1) {
            ++Data_CityInfo.housesRequiringReligionToEvolve;
            return 0;
        } else if (religion == 2) {
            ++Data_CityInfo.housesRequiringMoreReligionToEvolve;
            return 0;
        } else if (religion == 3) {
            ++Data_CityInfo.housesRequiringEvenMoreReligionToEvolve;
            return 0;
        }
    } else if (religion > 0) {
        ++Data_CityInfo.housesRequiringReligion;
    }
    // barber
    int barber = model->barber;
    if (house->data.house.barber < barber) {
        ++Data_CityInfo.housesRequiringBarberToEvolve;
        return 0;
    }
    if (barber == 1) {
        ++Data_CityInfo.housesRequiringBarber;
    }
    // bathhouse
    int bathhouse = model->bathhouse;
    if (house->data.house.bathhouse < bathhouse) {
        ++Data_CityInfo.housesRequiringBathhouseToEvolve;
        return 0;
    }
    if (bathhouse == 1) {
        ++Data_CityInfo.housesRequiringBathhouse;
    }
    // health
    int health = model->health;
    if (house->data.house.health < health) {
        if (health < 2) {
            ++Data_CityInfo.housesRequiringClinicToEvolve;
        } else {
            ++Data_CityInfo.housesRequiringHospitalToEvolve;
        }
        return 0;
    }
    if (health >= 1) {
        ++Data_CityInfo.housesRequiringClinic;
    }
    // food types
    int foodtypes_required = model->food_types;
    int foodtypes_available = 0;
    for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
        if (house->data.house.inventory[i]) {
            foodtypes_available++;
        }
    }
    if (foodtypes_available < foodtypes_required) {
        ++Data_CityInfo.housesRequiringFoodToEvolve;
        return 0;
    }
    // goods
    if (house->data.house.inventory[INVENTORY_POTTERY] < model->pottery) {
        return 0;
    }
    if (house->data.house.inventory[INVENTORY_OIL] < model->oil) {
        return 0;
    }
    if (house->data.house.inventory[INVENTORY_FURNITURE] < model->furniture) {
        return 0;
    }
    int wine = model->wine;
    if (wine && house->data.house.inventory[INVENTORY_WINE] <= 0) {
        return 0;
    }
    if (wine >= 2 && Data_CityInfo.resourceWineTypesAvailable < 2) {
        ++Data_CityInfo.housesRequiringSecondWineToEvolve;
        return 0;
    }
    return 1;
}

static int check_requirements(building *house)
{
    int status = check_evolve_desirability(house);
    if (!has_required_goods_and_services(house, 0)) {
        status = DEVOLVE;
    } else if (status == EVOLVE) {
        status = has_required_goods_and_services(house, 1);
    }
    return status;
}

static int has_devolve_delay(building *house, evolve_status status)
{
    if (status == DEVOLVE && house->data.house.devolveDelay < 2) {
        house->data.house.devolveDelay++;
        return 1;
    } else {
        house->data.house.devolveDelay = 0;
        return 0;
    }
}

static int evolve_small_tent(building *house)
{
    if (house->housePopulation > 0) {
        building_house_merge(house);
        evolve_status status = check_requirements(house);
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LARGE_TENT);
        }
    }
    return 0;
}

static int evolve_large_tent(building *house)
{
    if (house->housePopulation > 0) {
        building_house_merge(house);
        evolve_status status = check_requirements(house);
        if (!has_devolve_delay(house, status)) {
            if (status == EVOLVE) {
                building_house_change_to(house, BUILDING_HOUSE_SMALL_SHACK);
            } else if (status == DEVOLVE) {
                building_house_change_to(house, BUILDING_HOUSE_SMALL_TENT);
            }
        }
    }
    return 0;
}

static int evolve_small_shack(building *house)
{
    building_house_merge(house);
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LARGE_SHACK);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LARGE_TENT);
        }
    }
    return 0;
}

static int evolve_large_shack(building *house)
{
    building_house_merge(house);
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_HOVEL);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_SHACK);
        }
    }
    return 0;
}

static int evolve_small_hovel(building *house)
{
    building_house_merge(house);
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LARGE_HOVEL);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LARGE_SHACK);
        }
    }
    return 0;
}

static int evolve_large_hovel(building *house)
{
    building_house_merge(house);
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_CASA);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_HOVEL);
        }
    }
    return 0;
}

static int evolve_small_casa(building *house)
{
    building_house_merge(house);
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LARGE_CASA);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LARGE_HOVEL);
        }
    }
    return 0;
}

static int evolve_large_casa(building *house)
{
    building_house_merge(house);
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_INSULA);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_CASA);
        }
    }
    return 0;
}

static int evolve_small_insula(building *house)
{
    building_house_merge(house);
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_MEDIUM_INSULA);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LARGE_CASA);
        }
    }
    return 0;
}

static int evolve_medium_insula(building *house)
{
    building_house_merge(house);
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            if (building_house_can_expand(house, 4)) {
                house->houseIsMerged = 0;
                building_house_expand_to_large_insula(house);
                map_tiles_update_all_gardens();
                return 1;
            }
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_INSULA);
        }
    }
    return 0;
}

static int evolve_large_insula(building *house)
{
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_GRAND_INSULA);
        } else if (status == DEVOLVE) {
            building_house_devolve_from_large_insula(house);
        }
    }
    return 0;
}

static int evolve_grand_insula(building *house)
{
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_VILLA);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LARGE_INSULA);
        }
    }
    return 0;
}

static int evolve_small_villa(building *house)
{
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_MEDIUM_VILLA);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_GRAND_INSULA);
        }
    }
    return 0;
}

static int evolve_medium_villa(building *house)
{
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            if (building_house_can_expand(house, 9)) {
                building_house_expand_to_large_villa(house);
                map_tiles_update_all_gardens();
                return 1;
            }
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_VILLA);
        }
    }
    return 0;
}

static int evolve_large_villa(building *house)
{
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_GRAND_VILLA);
        } else if (status == DEVOLVE) {
            building_house_devolve_from_large_villa(house);
        }
    }
    return 0;
}

static int evolve_grand_villa(building *house)
{
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_PALACE);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LARGE_VILLA);
        }
    }
    return 0;
}

static int evolve_small_palace(building *house)
{
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_MEDIUM_PALACE);
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_GRAND_VILLA);
        }
    }
    return 0;
}

static int evolve_medium_palace(building *house)
{
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            if (building_house_can_expand(house, 16)) {
                building_house_expand_to_large_palace(house);
                map_tiles_update_all_gardens();
                return 1;
            }
        } else if (status == DEVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_SMALL_PALACE);
        }
    }
    return 0;
}

static int evolve_large_palace(building *house)
{
    int status = check_requirements(house);
    if (!has_devolve_delay(house, status)) {
        if (status == EVOLVE) {
            building_house_change_to(house, BUILDING_HOUSE_LUXURY_PALACE);
        } else if (status == DEVOLVE) {
            building_house_devolve_from_large_palace(house);
        }
    }
    return 0;
}

static int evolve_luxury_palace(building *house)
{
    int status = check_evolve_desirability(house);
    if (!has_required_goods_and_services(house, 0)) {
        status = DEVOLVE;
    }
    if (!has_devolve_delay(house, status) && status == DEVOLVE) {
        building_house_change_to(house, BUILDING_HOUSE_LARGE_PALACE);
    }
    return 0;
}

static void consume_resource(building *b, int inventory, int amount)
{
    if (amount > 0) {
        if (amount > b->data.house.inventory[inventory]) {
            b->data.house.inventory[inventory] = 0;
        } else {
            b->data.house.inventory[inventory] -= amount;
        }
    }
}

static void consume_resources(building *b)
{
    const model_house *model = model_get_house(b->subtype.houseLevel);
    consume_resource(b, INVENTORY_POTTERY, model->pottery);
    consume_resource(b, INVENTORY_FURNITURE, model->furniture);
    consume_resource(b, INVENTORY_OIL, model->oil);
    consume_resource(b, INVENTORY_WINE, model->wine);
}

static int (*evolve_callback[])(building *) = {
    evolve_small_tent, evolve_large_tent, evolve_small_shack, evolve_large_shack,
    evolve_small_hovel, evolve_large_hovel, evolve_small_casa, evolve_large_casa,
    evolve_small_insula, evolve_medium_insula, evolve_large_insula, evolve_grand_insula,
    evolve_small_villa, evolve_medium_villa, evolve_large_villa, evolve_grand_villa,
    evolve_small_palace, evolve_medium_palace, evolve_large_palace, evolve_luxury_palace
};

void building_house_process_evolve_and_consume_goods()
{
    reset_service_required_counters();
    int has_expanded = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && building_is_house(b->type)) {
            building_house_check_for_corruption(b);
            has_expanded |= evolve_callback[b->type - BUILDING_HOUSE_VACANT_LOT](b);
            if (game_time_day() == 0 || game_time_day() == 7) {
                consume_resources(b);
            }
        }
    }
    if (has_expanded) {
        map_routing_update_land();
    }
}

void building_house_determine_evolve_text(building *house, int worst_desirability_building)
{
    int level = house->subtype.houseLevel;
    
    // this house will devolve soon because...

    const model_house *model = model_get_house(level);
    // desirability
    if (house->desirability <= model->devolve_desirability) {
        house->data.house.evolveTextId = 0;
        return;
    }
    // water
    int water = model->water;
    if (water == 1 && !house->hasWaterAccess && !house->hasWellAccess) {
        house->data.house.evolveTextId = 1;
        return;
    }
    if (water == 2 && !house->hasWaterAccess) {
        house->data.house.evolveTextId = 2;
        return;
    }
    // entertainment
    int entertainment = model->entertainment;
    if (house->data.house.entertainment < entertainment) {
        if (!house->data.house.entertainment) {
            house->data.house.evolveTextId = 3;
        } else if (entertainment < 10) {
            house->data.house.evolveTextId = 4;
        } else if (entertainment < 25) {
            house->data.house.evolveTextId = 5;
        } else if (entertainment < 50) {
            house->data.house.evolveTextId = 6;
        } else if (entertainment < 80) {
            house->data.house.evolveTextId = 7;
        } else {
            house->data.house.evolveTextId = 8;
        }
        return;
    }
    // food types
    int foodtypes_required = model->food;
    int foodtypes_available = 0;
    for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
        if (house->data.house.inventory[i]) {
            foodtypes_available++;
        }
    }
    if (foodtypes_available < foodtypes_required) {
        if (foodtypes_required == 1) {
            house->data.house.evolveTextId = 9;
            return;
        } else if (foodtypes_required == 2) {
            house->data.house.evolveTextId = 10;
            return;
        } else if (foodtypes_required == 3) {
            house->data.house.evolveTextId = 11;
            return;
        }
    }
    // education
    int education = model->education;
    if (house->data.house.education < education) {
        if (education == 1) {
            house->data.house.evolveTextId = 14;
            return;
        } else if (education == 2) {
            if (house->data.house.school) {
                house->data.house.evolveTextId = 15;
                return;
            } else if (house->data.house.library) {
                house->data.house.evolveTextId = 16;
                return;
            }
        } else if (education == 3) {
            house->data.house.evolveTextId = 17;
            return;
        }
    }
    // bathhouse
    if (house->data.house.bathhouse < model->bathhouse) {
        house->data.house.evolveTextId = 18;
        return;
    }
    // pottery
    if (house->data.house.inventory[INVENTORY_POTTERY] < model->pottery) {
        house->data.house.evolveTextId = 19;
        return;
    }
    // religion
    int religion = model->religion;
    if (house->data.house.numGods < religion) {
        if (religion == 1) {
            house->data.house.evolveTextId = 20;
            return;
        } else if (religion == 2) {
            house->data.house.evolveTextId = 21;
            return;
        } else if (religion == 3) {
            house->data.house.evolveTextId = 22;
            return;
        }
    }
    // barber
    if (house->data.house.barber < model->barber) {
        house->data.house.evolveTextId = 23;
        return;
    }
    // health
    int health = model->health;
    if (house->data.house.health < health) {
        if (health == 1) {
            house->data.house.evolveTextId = 24;
        } else if (house->data.house.clinic) {
            house->data.house.evolveTextId = 25;
        } else {
            house->data.house.evolveTextId = 26;
        }
        return;
    }
    // oil
    if (house->data.house.inventory[INVENTORY_OIL] < model->oil) {
        house->data.house.evolveTextId = 27;
        return;
    }
    // furniture
    if (house->data.house.inventory[INVENTORY_FURNITURE] < model->furniture) {
        house->data.house.evolveTextId = 28;
        return;
    }
    // wine
    int wine = model->wine;
    if (house->data.house.inventory[INVENTORY_WINE] < wine) {
        house->data.house.evolveTextId = 29;
        return;
    }
    if (wine > 1 && Data_CityInfo.resourceWineTypesAvailable < 2) {
        house->data.house.evolveTextId = 65;
        return;
    }
    if (level >= 19) { // max level!
        house->data.house.evolveTextId = 60;
        return;
    }

    // this house will evolve if ...

    // desirability
    if (house->desirability < model->evolve_desirability) {
        if (worst_desirability_building) {
            house->data.house.evolveTextId = 62;
        } else {
            house->data.house.evolveTextId = 30;
        }
        return;
    }
    model = model_get_house(++level);
    // water
    water = model->water;
    if (water == 1 && !house->hasWaterAccess && !house->hasWellAccess) {
        house->data.house.evolveTextId = 31;
        return;
    }
    if (water == 2 && !house->hasWaterAccess) {
        house->data.house.evolveTextId = 32;
        return;
    }
    // entertainment
    entertainment = model->entertainment;
    if (house->data.house.entertainment < entertainment) {
        if (!house->data.house.entertainment) {
            house->data.house.evolveTextId = 33;
        } else if (entertainment < 10) {
            house->data.house.evolveTextId = 34;
        } else if (entertainment < 25) {
            house->data.house.evolveTextId = 35;
        } else if (entertainment < 50) {
            house->data.house.evolveTextId = 36;
        } else if (entertainment < 80) {
            house->data.house.evolveTextId = 37;
        } else {
            house->data.house.evolveTextId = 38;
        }
        return;
    }
    // food types
    foodtypes_required = model->food_types;
    if (foodtypes_available < foodtypes_required) {
        if (foodtypes_required == 1) {
            house->data.house.evolveTextId = 39;
            return;
        } else if (foodtypes_required == 2) {
            house->data.house.evolveTextId = 40;
            return;
        } else if (foodtypes_required == 3) {
            house->data.house.evolveTextId = 41;
            return;
        }
    }
    // education
    education = model->education;
    if (house->data.house.education < education) {
        if (education == 1) {
            house->data.house.evolveTextId = 44;
            return;
        } else if (education == 2) {
            if (house->data.house.school) {
                house->data.house.evolveTextId = 45;
                return;
            } else if (house->data.house.library) {
                house->data.house.evolveTextId = 46;
                return;
            }
        } else if (education == 3) {
            house->data.house.evolveTextId = 47;
            return;
        }
    }
    // bathhouse
    if (house->data.house.bathhouse < model->bathhouse) {
        house->data.house.evolveTextId = 48;
        return;
    }
    // pottery
    if (house->data.house.inventory[INVENTORY_POTTERY] < model->pottery) {
        house->data.house.evolveTextId = 49;
        return;
    }
    // religion
    religion = model->religion;
    if (house->data.house.numGods < religion) {
        if (religion == 1) {
            house->data.house.evolveTextId = 50;
            return;
        } else if (religion == 2) {
            house->data.house.evolveTextId = 51;
            return;
        } else if (religion == 3) {
            house->data.house.evolveTextId = 52;
            return;
        }
    }
    // barber
    if (house->data.house.barber < model->barber) {
        house->data.house.evolveTextId = 53;
        return;
    }
    // health
    health = model->health;
    if (house->data.house.health < health) {
        if (health == 1) {
            house->data.house.evolveTextId = 54;
        } else if (house->data.house.clinic) {
            house->data.house.evolveTextId = 55;
        } else {
            house->data.house.evolveTextId = 56;
        }
        return;
    }
    // oil
    if (house->data.house.inventory[INVENTORY_OIL] < model->oil) {
        house->data.house.evolveTextId = 57;
        return;
    }
    // furniture
    if (house->data.house.inventory[INVENTORY_FURNITURE] < model->furniture) {
        house->data.house.evolveTextId = 58;
        return;
    }
    // wine
    wine = model->wine;
    if (house->data.house.inventory[INVENTORY_WINE] < wine) {
        house->data.house.evolveTextId = 59;
        return;
    }
    if (wine > 1 && Data_CityInfo.resourceWineTypesAvailable < 2) {
        house->data.house.evolveTextId = 66;
        return;
    }
    // house is evolving
    house->data.house.evolveTextId = 61;
    if (house->data.house.noSpaceToExpand == 1) {
        // house would like to evolve but can't
        house->data.house.evolveTextId = 64;
    }
}

int building_house_determine_worst_desirability_building(const building *house)
{
    int lowest_desirability = 0;
    int lowest_building_id = 0;
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(house->x, house->y, 1, 6, &x_min, &y_min, &x_max, &y_max);

    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int building_id = map_building_at(map_grid_offset(x, y));
            if (building_id <= 0) {
                continue;
            }
            building *b = building_get(building_id);
            if (b->state != BUILDING_STATE_IN_USE || building_id == house->id) {
                continue;
            }
            if (!b->houseSize || b->type < house->type) {
                int des = model_get_building(b->type)->desirability_value;
                if (des < 0) {
                    // simplified desirability calculation
                    int step_size = model_get_building(b->type)->desirability_step_size;
                    int range = model_get_building(b->type)->desirability_range;
                    int dist = calc_maximum_distance(x, y, house->x, house->y);
                    if (dist <= range) {
                        while (--dist > 1) {
                            des += step_size;
                        }
                        if (des < lowest_desirability) {
                            lowest_desirability = des;
                            lowest_building_id = building_id;
                        }
                    }
                }
            }
        }
    }
    return lowest_building_id;
}
