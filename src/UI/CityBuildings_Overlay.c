#include "CityBuildings_private.h"

#include "building/animation.h"
#include "building/industry.h"
#include "city/view.h"
#include "figure/figure.h"
#include "game/resource.h"
#include "game/state.h"
#include "map/desirability.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"

static void drawFootprintForWaterOverlay(int gridOffset, int xOffset, int yOffset);
static void drawTopForWaterOverlay(int gridOffset, int xOffset, int yOffset);
static void drawFootprintForNativeOverlay(int gridOffset, int xOffset, int yOffset);
static void drawTopForNativeOverlay(int gridOffset, int xOffset, int yOffset);
static void drawBuildingFootprintForOverlay(int buildingId, int gridOffset, int xOffset, int yOffset, int graphicOffset);
static void drawBuildingFootprintForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset);
static void drawBuildingTopForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset);
static void drawBuildingTopForFireOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForDamageOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForCrimeOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForEntertainmentOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForEducationOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForTheaterOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForAmphitheaterOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForColosseumOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForHippodromeOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForFoodStocksOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForBathhouseOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForReligionOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForSchoolOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForLibraryOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForAcademyOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForBarberOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForClinicsOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForHospitalOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForTaxIncomeOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForProblemsOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawOverlayColumn(int height, int xOffset, int yOffset, int isRed);

static void draw_foot_with_size(int grid_offset, int image_x, int image_y)
{
    int image_id = map_image_at(grid_offset);
    switch (map_property_multi_tile_size(grid_offset)) {
        case 1:
            DRAWFOOT_SIZE1(image_id, image_x, image_y);
            break;
        case 2:
            DRAWFOOT_SIZE2(image_id, image_x, image_y);
            break;
        case 3:
            DRAWFOOT_SIZE3(image_id, image_x, image_y);
            break;
        case 4:
            DRAWFOOT_SIZE4(image_id, image_x, image_y);
            break;
        case 5:
            DRAWFOOT_SIZE5(image_id, image_x, image_y);
            break;
    }
}

static void draw_top_with_size(int grid_offset, int image_x, int image_y)
{
    int image_id = map_image_at(grid_offset);
    switch (map_property_multi_tile_size(grid_offset)) {
        case 1:
            DRAWTOP_SIZE1(image_id, image_x, image_y);
            break;
        case 2:
            DRAWTOP_SIZE2(image_id, image_x, image_y);
            break;
        case 3:
            DRAWTOP_SIZE3(image_id, image_x, image_y);
            break;
        case 4:
            DRAWTOP_SIZE4(image_id, image_x, image_y);
            break;
        case 5:
            DRAWTOP_SIZE5(image_id, image_x, image_y);
            break;
    }
}

void UI_CityBuildings_drawOverlayFootprints()
{
    int overlay = game_state_overlay();
	FOREACH_XY_VIEW {
		int gridOffset = ViewToGridOffset(xView, yView);
		if (gridOffset == Data_State.selectedBuilding.gridOffsetStart) {
			Data_State.selectedBuilding.reservoirOffsetX = xGraphic;
			Data_State.selectedBuilding.reservoirOffsetY = yGraphic;
		}
		if (gridOffset < 0) {
			// Outside map: draw black tile
			DRAWFOOT_SIZE1(image_group(GROUP_TERRAIN_BLACK),
				xGraphic, yGraphic);
		} else if (overlay == OVERLAY_DESIRABILITY) {
			drawBuildingFootprintForDesirabilityOverlay(gridOffset, xGraphic, yGraphic);
		} else if (map_property_is_draw_tile(gridOffset)) {
			int terrain = map_terrain_get(gridOffset);
			if (overlay == OVERLAY_WATER) {
				drawFootprintForWaterOverlay(gridOffset, xGraphic, yGraphic);
			} else if (overlay == OVERLAY_NATIVE) {
				drawFootprintForNativeOverlay(gridOffset, xGraphic, yGraphic);
			} else if (terrain & (TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
				// display grass
				int graphicId = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7);
				DRAWFOOT_SIZE1(graphicId, xGraphic, yGraphic);
			} else if ((terrain & TERRAIN_ROAD) && !(terrain & TERRAIN_BUILDING)) {
				draw_foot_with_size(gridOffset, xGraphic, yGraphic);
			} else if (terrain & TERRAIN_BUILDING) {
				drawBuildingFootprintForOverlay(map_building_at(gridOffset),
					gridOffset, xGraphic, yGraphic, 0);
			} else {
				draw_foot_with_size(gridOffset, xGraphic, yGraphic);
			}
		}
	} END_FOREACH_XY_VIEW;
}

void UI_CityBuildings_drawOverlayTopsFiguresAnimation(int overlay)
{
	FOREACH_Y_VIEW {
		// draw figures
		FOREACH_X_VIEW {
			int figureId = map_figure_at(gridOffset);
			while (figureId) {
			    figure *fig = figure_get(figureId);
				if (!fig->isGhost) {
					UI_CityBuildings_drawFigure(figureId, xGraphic, yGraphic, 9999, 0);
				}
				figureId = fig->nextFigureIdOnSameTile;
			}
		} END_FOREACH_X_VIEW;
		// draw animation
		FOREACH_X_VIEW {
			if (overlay == OVERLAY_DESIRABILITY) {
				drawBuildingTopForDesirabilityOverlay(gridOffset, xGraphic, yGraphic);
			} else if (map_property_is_draw_tile(gridOffset)) {
				if (overlay == OVERLAY_WATER) {
					drawTopForWaterOverlay(gridOffset, xGraphic, yGraphic);
				} else if (overlay == OVERLAY_NATIVE) {
					drawTopForNativeOverlay(gridOffset, xGraphic, yGraphic);
				} else if (!map_terrain_is(gridOffset, TERRAIN_WALL | TERRAIN_AQUEDUCT | TERRAIN_ROAD)) {
					if (map_terrain_is(gridOffset, TERRAIN_BUILDING) && map_building_at(gridOffset)) {
						building *b = building_get(map_building_at(gridOffset));
						switch (overlay) {
							case OVERLAY_FIRE:
								drawBuildingTopForFireOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_DAMAGE:
								drawBuildingTopForDamageOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_CRIME:
								drawBuildingTopForCrimeOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_ENTERTAINMENT:
								drawBuildingTopForEntertainmentOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_THEATER:
								drawBuildingTopForTheaterOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_AMPHITHEATER:
								drawBuildingTopForAmphitheaterOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_COLOSSEUM:
								drawBuildingTopForColosseumOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_HIPPODROME:
								drawBuildingTopForHippodromeOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_RELIGION:
								drawBuildingTopForReligionOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_EDUCATION:
								drawBuildingTopForEducationOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_SCHOOL:
								drawBuildingTopForSchoolOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_LIBRARY:
								drawBuildingTopForLibraryOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_ACADEMY:
								drawBuildingTopForAcademyOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_BARBER:
								drawBuildingTopForBarberOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_BATHHOUSE:
								drawBuildingTopForBathhouseOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_CLINIC:
								drawBuildingTopForClinicsOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_HOSPITAL:
								drawBuildingTopForHospitalOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_FOOD_STOCKS:
								drawBuildingTopForFoodStocksOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_TAX_INCOME:
								drawBuildingTopForTaxIncomeOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
							case OVERLAY_PROBLEMS:
								drawBuildingTopForProblemsOverlay(gridOffset, b, xGraphic, yGraphic);
								break;
						}
					} else if (!map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
						// terrain
						draw_top_with_size(gridOffset, xGraphic, yGraphic);
					}
				}
			}
		} END_FOREACH_X_VIEW;
		FOREACH_X_VIEW {
			int draw = 0;
			if (map_building_at(gridOffset)) {
				int btype = building_get(map_building_at(gridOffset))->type;
				switch (overlay) {
					case OVERLAY_FIRE:
					case OVERLAY_CRIME:
						if (btype == BUILDING_PREFECTURE || btype == BUILDING_BURNING_RUIN) {
							draw = 1;
						}
						break;
					case OVERLAY_DAMAGE:
						if (btype == BUILDING_ENGINEERS_POST) {
							draw = 1;
						}
						break;
					case OVERLAY_WATER:
						if (btype == BUILDING_RESERVOIR || btype == BUILDING_FOUNTAIN) {
							draw = 1;
						}
						break;
					case OVERLAY_FOOD_STOCKS:
						if (btype == BUILDING_MARKET || btype == BUILDING_GRANARY) {
							draw = 1;
						}
						break;
				}
			}

			int graphicId = map_image_at(gridOffset);
			const image *img = image_get(graphicId);
			if (img->num_animation_sprites && draw) {
				if (map_property_is_draw_tile(gridOffset)) {
					building *b = building_get(map_building_at(gridOffset));
					int colorMask = 0;
					if (b->type == BUILDING_GRANARY) {
						image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 1,
							xGraphic + img->sprite_offset_x,
							yGraphic + 60 + img->sprite_offset_y - img->height,
							colorMask);
						if (b->data.storage.resourceStored[RESOURCE_NONE] < 2400) {
							image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 2,
								xGraphic + 33, yGraphic - 60, colorMask);
						}
						if (b->data.storage.resourceStored[RESOURCE_NONE] < 1800) {
							image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 3,
								xGraphic + 56, yGraphic - 50, colorMask);
						}
						if (b->data.storage.resourceStored[RESOURCE_NONE] < 1200) {
							image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 4,
								xGraphic + 91, yGraphic - 50, colorMask);
						}
						if (b->data.storage.resourceStored[RESOURCE_NONE] < 600) {
							image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 5,
								xGraphic + 117, yGraphic - 62, colorMask);
						}
					} else {
						int animationOffset = building_animation_offset(b, graphicId, gridOffset);
						if (animationOffset > 0) {
							if (animationOffset > img->num_animation_sprites) {
								animationOffset = img->num_animation_sprites;
							}
							int ydiff = 0;
							switch (map_property_multi_tile_size(gridOffset)) {
								case 1: ydiff = 30; break;
								case 2: ydiff = 45; break;
								case 3: ydiff = 60; break;
								case 4: ydiff = 75; break;
								case 5: ydiff = 90; break;
							}
							image_draw_masked(graphicId + animationOffset,
								xGraphic + img->sprite_offset_x,
								yGraphic + ydiff + img->sprite_offset_y - img->height,
								colorMask);
						}
					}
				}
			} else if (map_is_bridge(gridOffset)) {
				UI_CityBuildings_drawBridge(gridOffset, xGraphic, yGraphic);
			}
		} END_FOREACH_X_VIEW;
	} END_FOREACH_Y_VIEW;
}

static int terrain_on_water_overlay()
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SCRUB |
        TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_AQUEDUCT | TERRAIN_ELEVATION |
        TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}

static void drawFootprintForWaterOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_water_overlay())) {
		if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
			drawBuildingFootprintForOverlay(map_building_at(gridOffset),
				gridOffset, xOffset, yOffset, 0);
		} else {
			draw_foot_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_WALL)) {
		// display grass
		int graphicId = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7);
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
        building *b = building_get(map_building_at(gridOffset));
		int terrain = map_terrain_get(gridOffset);
		if (b->id && b->hasWellAccess == 1) {
			terrain |= TERRAIN_FOUNTAIN_RANGE;
		}
		if (b->type == BUILDING_WELL || b->type == BUILDING_FOUNTAIN) {
			DRAWFOOT_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
		} else if (b->type == BUILDING_RESERVOIR) {
			DRAWFOOT_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
		} else {
			int graphicOffset;
			switch (terrain & (TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE)) {
				case TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE:
					graphicOffset = 24;
					break;
				case TERRAIN_RESERVOIR_RANGE:
					graphicOffset = 8;
					break;
				case TERRAIN_FOUNTAIN_RANGE:
					graphicOffset = 16;
					break;
				default:
					graphicOffset = 0;
					break;
			}
			drawBuildingFootprintForOverlay(b->id, gridOffset, xOffset, yOffset, graphicOffset);
		}
	} else {
		int graphicId = image_group(GROUP_TERRAIN_OVERLAY);
		switch (map_terrain_get(gridOffset) & (TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE)) {
			case TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE:
				graphicId += 27;
				break;
			case TERRAIN_RESERVOIR_RANGE:
				graphicId += 11;
				break;
			case TERRAIN_FOUNTAIN_RANGE:
				graphicId += 19;
				break;
			default:
				graphicId = map_image_at(gridOffset);
				break;
		}
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	}
}

static void drawTopForWaterOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_water_overlay())) {
		if (!map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
			draw_top_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_building_at(gridOffset)) {
		building *b = building_get(map_building_at(gridOffset));
		if (b->type == BUILDING_WELL || b->type == BUILDING_FOUNTAIN) {
			DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
		} else if (b->type == BUILDING_RESERVOIR) {
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
		}
	}
}

static int terrain_on_native_overlay()
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SCRUB |
        TERRAIN_GARDEN | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}

static void drawFootprintForNativeOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_native_overlay())) {
		if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
			drawBuildingFootprintForOverlay(map_building_at(gridOffset),
				gridOffset, xOffset, yOffset, 0);
		} else {
			draw_foot_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
		// display grass
		int graphicId = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7);
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
		drawBuildingFootprintForOverlay(map_building_at(gridOffset),
		gridOffset, xOffset, yOffset, 0);
	} else {
		if (map_property_is_native_land(gridOffset)) {
			DRAWFOOT_SIZE1(image_group(GROUP_TERRAIN_DESIRABILITY) + 1, xOffset, yOffset);
		} else {
			draw_foot_with_size(gridOffset, xOffset, yOffset);
		}
	}
}

static void drawTopForNativeOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_native_overlay())) {
		if (!map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
			draw_top_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_building_at(gridOffset)) {
		int graphicId = map_image_at(gridOffset);
		switch (building_get(map_building_at(gridOffset))->type) {
			case BUILDING_NATIVE_HUT:
				DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
				break;
			case BUILDING_NATIVE_MEETING:
			case BUILDING_MISSION_POST:
				DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
				break;
		}
	}
}

static int is_drawable_farmhouse(int grid_offset, int map_orientation)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return 0;
    }
    int xy = map_property_multi_tile_xy(grid_offset);
    if (map_orientation == DIR_0_TOP && xy == Edge_X0Y1) {
        return 1;
    }
    if (map_orientation == DIR_2_RIGHT && xy == Edge_X0Y0) {
        return 1;
    }
    if (map_orientation == DIR_4_BOTTOM && xy == Edge_X1Y0) {
        return 1;
    }
    if (map_orientation == DIR_2_RIGHT && xy == Edge_X1Y1) {
        return 1;
    }
    return 0;
}

static int is_drawable_farm_corner(int grid_offset, int map_orientation)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return 0;
    }

    int xy = map_property_multi_tile_xy(grid_offset);
    if (map_orientation == DIR_0_TOP && xy == Edge_X0Y2) {
        return 1;
    }
    if (map_orientation == DIR_2_RIGHT && xy == Edge_X0Y0) {
        return 1;
    }
    if (map_orientation == DIR_4_BOTTOM && xy == Edge_X2Y0) {
        return 1;
    }
    if (map_orientation == DIR_2_RIGHT && xy == Edge_X2Y2) {
        return 1;
    }
    return 0;
}

static void drawBuildingFootprintForOverlay(int buildingId, int gridOffset, int xOffset, int yOffset, int graphicOffset)
{
	if (!buildingId) {
		return;
	}
	building *b = building_get(buildingId);
	int origGraphicId = map_image_at(gridOffset);
	if (b->size == 1) {
		int graphicId = image_group(GROUP_TERRAIN_OVERLAY);
		if (b->houseSize) {
			graphicId += 4;
		}
		switch (game_state_overlay()) {
			case OVERLAY_DAMAGE:
				if (b->type == BUILDING_ENGINEERS_POST) {
					graphicId = origGraphicId;
				}
				break;
			case OVERLAY_BARBER:
				if (b->type == BUILDING_BARBER) {
					graphicId = origGraphicId;
				}
				break;
			case OVERLAY_CLINIC:
				if (b->type == BUILDING_DOCTOR) {
					graphicId = origGraphicId;
				}
				break;
			case OVERLAY_NATIVE:
				if (b->type == BUILDING_NATIVE_HUT) {
					graphicId = origGraphicId;
					graphicOffset = 0;
				}
				break;
			case OVERLAY_PROBLEMS:
				if (b->showOnProblemOverlay) {
					graphicId = origGraphicId;
				}
				break;
			case OVERLAY_FIRE:
			case OVERLAY_CRIME:
				if (b->type == BUILDING_PREFECTURE || b->type == BUILDING_BURNING_RUIN) {
					graphicId = origGraphicId;
				}
				break;
		}
		graphicId += graphicOffset;
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (b->size == 2) {
		int drawOrig = 0;
		switch (game_state_overlay()) {
			case OVERLAY_ENTERTAINMENT:
			case OVERLAY_THEATER:
				if (b->type == BUILDING_THEATER) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_EDUCATION:
				if (b->type == BUILDING_SCHOOL || b->type == BUILDING_LIBRARY) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_SCHOOL:
				if (b->type == BUILDING_SCHOOL) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_LIBRARY:
				if (b->type == BUILDING_LIBRARY) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_BATHHOUSE:
				if (b->type == BUILDING_BATHHOUSE) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_RELIGION:
				if (b->type == BUILDING_ORACLE || b->type == BUILDING_SMALL_TEMPLE_CERES ||
					b->type == BUILDING_SMALL_TEMPLE_NEPTUNE || b->type == BUILDING_SMALL_TEMPLE_MERCURY ||
					b->type == BUILDING_SMALL_TEMPLE_MARS || b->type == BUILDING_SMALL_TEMPLE_VENUS) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_FOOD_STOCKS:
				if (b->type == BUILDING_MARKET || b->type == BUILDING_WHARF) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_TAX_INCOME:
				if (b->type == BUILDING_FORUM) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_NATIVE:
				if (b->type == BUILDING_NATIVE_MEETING || b->type == BUILDING_MISSION_POST) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_PROBLEMS:
				if (b->showOnProblemOverlay) {
					drawOrig = 1;
				}
				break;
		}
		if (drawOrig) {
			DRAWFOOT_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
		} else {
			int graphicBase = image_group(GROUP_TERRAIN_OVERLAY) + graphicOffset;
			if (b->houseSize) {
				graphicBase += 4;
			}
			int xTileOffset[] = {30, 0, 60, 30};
			int yTileOffset[] = {-15, 0, 0, 15};
			for (int i = 0; i < 4; i++) {
				image_draw_isometric_footprint(graphicBase + i,
					xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
			}
		}
	} else if (b->size == 3) {
		int drawOrig = 0;
		switch (game_state_overlay()) {
			case OVERLAY_ENTERTAINMENT:
				if (b->type == BUILDING_AMPHITHEATER || b->type == BUILDING_GLADIATOR_SCHOOL ||
					b->type == BUILDING_LION_HOUSE || b->type == BUILDING_ACTOR_COLONY ||
					b->type == BUILDING_CHARIOT_MAKER) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_THEATER:
				if (b->type == BUILDING_ACTOR_COLONY) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_AMPHITHEATER:
				if (b->type == BUILDING_ACTOR_COLONY || b->type == BUILDING_GLADIATOR_SCHOOL ||
					b->type == BUILDING_AMPHITHEATER) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_COLOSSEUM:
				if (b->type == BUILDING_GLADIATOR_SCHOOL || b->type == BUILDING_LION_HOUSE) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_HIPPODROME:
				if (b->type == BUILDING_CHARIOT_MAKER) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_EDUCATION:
			case OVERLAY_ACADEMY:
				if (b->type == BUILDING_ACADEMY) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_HOSPITAL:
				if (b->type == BUILDING_HOSPITAL) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_RELIGION:
				if (b->type == BUILDING_LARGE_TEMPLE_CERES || b->type == BUILDING_LARGE_TEMPLE_NEPTUNE ||
					b->type == BUILDING_LARGE_TEMPLE_MERCURY || b->type == BUILDING_LARGE_TEMPLE_MARS ||
					b->type == BUILDING_LARGE_TEMPLE_VENUS) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_FOOD_STOCKS:
				if (b->type == BUILDING_GRANARY) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_PROBLEMS:
				if (b->showOnProblemOverlay) {
					drawOrig = 1;
				}
				break;
		}
		// farms have multiple drawable tiles: the farmhouse and 5 fields
		if (drawOrig) {
			if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_PIG_FARM) {
				if (is_drawable_farmhouse(gridOffset, city_view_orientation())) {
					DRAWFOOT_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
				} else if (map_property_is_draw_tile(gridOffset)) {
					DRAWFOOT_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
				}
			} else {
				DRAWFOOT_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			}
		} else {
			int draw = 1;
			if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_PIG_FARM) {
			    draw = is_drawable_farm_corner(gridOffset, city_view_orientation());
			}
			if (draw) {
				int graphicBase = image_group(GROUP_TERRAIN_OVERLAY) + graphicOffset;
				if (b->houseSize) {
					graphicBase += 4;
				}
				int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 3, 3, 3};
				int xTileOffset[] = {60, 30, 90, 0, 60, 120, 30, 90, 60};
				int yTileOffset[] = {-30, -15, -15, 0, 0, 0, 15, 15, 30};
				for (int i = 0; i < 9; i++) {
					image_draw_isometric_footprint(graphicBase + graphicTileOffset[i],
						xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
				}
			}
		}
	} else if (b->size == 4) {
		int graphicBase = image_group(GROUP_TERRAIN_OVERLAY) + graphicOffset;
		if (b->houseSize) {
			graphicBase += 4;
		}
		int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 1, 3, 3, 2, 3, 3, 3, 3, 3, 3};
		int xTileOffset[] = {
			90,
			60, 120,
			30, 90, 150,
			0, 60, 120, 180,
			30, 90, 150,
			60, 120,
			90
		};
		int yTileOffset[] = {
			-45,
			-30, -30,
			-15, -15, -15,
			0, 0, 0, 0,
			15, 15, 15,
			30, 30,
			45
		};
		for (int i = 0; i < 16; i++) {
			image_draw_isometric_footprint(graphicBase + graphicTileOffset[i],
				xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
		}
	} else if (b->size == 5) {
		int drawOrig = 0;
		switch (game_state_overlay()) {
			case OVERLAY_ENTERTAINMENT:
				if (b->type == BUILDING_HIPPODROME || b->type == BUILDING_COLOSSEUM) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_COLOSSEUM:
				if (b->type == BUILDING_COLOSSEUM) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_HIPPODROME:
				if (b->type == BUILDING_HIPPODROME) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_TAX_INCOME:
				if (b->type == BUILDING_SENATE_UPGRADED) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_PROBLEMS:
				if (b->showOnProblemOverlay) {
					drawOrig = 1;
				}
				break;
		}
		if (drawOrig) {
			DRAWFOOT_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
		} else {
			int graphicBase = image_group(GROUP_TERRAIN_OVERLAY) + graphicOffset;
			if (b->houseSize) {
				graphicBase += 4;
			}
			int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 1, 3, 3, 2,
				1, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
			int xTileOffset[] = {
				120,
				90, 150,
				60, 120, 180,
				30, 90, 150, 210,
				0, 60, 120, 180, 240,
				30, 90, 150, 210,
				60, 120, 180,
				90, 150,
				120
			};
			int yTileOffset[] = {
				-60,
				-45, -45,
				-30, -30, -30,
				-15, -15, -15, -15,
				0, 0, 0, 0, 0,
				15, 15, 15, 15,
				30, 30, 30,
				45, 45,
				60
			};
			for (int i = 0; i < 25; i++) {
				image_draw_isometric_footprint(graphicBase + graphicTileOffset[i],
					xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
			}
		}
	}
}

static int terrain_on_desirability_overlay()
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER |
        TERRAIN_SCRUB | TERRAIN_GARDEN | TERRAIN_ROAD |
        TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}

static void drawBuildingFootprintForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_desirability_overlay()) && !map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
		// display normal tile
		if (map_property_is_draw_tile(gridOffset)) {
			draw_foot_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
		// display empty land/grass
		int graphicId = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7);
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (map_terrain_is(gridOffset, TERRAIN_BUILDING) || map_desirability_get(gridOffset)) {
		int des = map_desirability_get(gridOffset);
		int offset = 0;
		if (des < -10) {
			offset = 0;
		} else if (des < -5) {
			offset = 1;
		} else if (des < 0) {
			offset = 2;
		} else if (des == 1) {
			offset = 3;
		} else if (des < 5) {
			offset = 4;
		} else if (des < 10) {
			offset = 5;
		} else if (des < 15) {
			offset = 6;
		} else if (des < 20) {
			offset = 7;
		} else if (des < 25) {
			offset = 8;
		} else {
			offset = 9;
		}
		DRAWFOOT_SIZE1(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, xOffset, yOffset);
	} else {
		DRAWFOOT_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	}
}

static void drawBuildingTopForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_desirability_overlay()) && !map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
		// display normal tile
		if (map_property_is_draw_tile(gridOffset)) {
			draw_top_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
		// grass, no top needed
	} else if (map_terrain_is(gridOffset, TERRAIN_BUILDING) || map_desirability_get(gridOffset)) {
		int des = map_desirability_get(gridOffset);
		int offset;
		if (des < -10) {
			offset = 0;
		} else if (des < -5) {
			offset = 1;
		} else if (des < 0) {
			offset = 2;
		} else if (des == 1) {
			offset = 3;
		} else if (des < 5) {
			offset = 4;
		} else if (des < 10) {
			offset = 5;
		} else if (des < 15) {
			offset = 6;
		} else if (des < 20) {
			offset = 7;
		} else if (des < 25) {
			offset = 8;
		} else {
			offset = 9;
		}
		DRAWTOP_SIZE1(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, xOffset, yOffset);
	} else {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	}
}

static void drawBuildingTopForFireOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_PREFECTURE) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->type == BUILDING_BURNING_RUIN) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->fireRisk > 0) {
		int draw = 1;
		if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_PIG_FARM) {
			draw = is_drawable_farm_corner(gridOffset, city_view_orientation());
		}
		if (draw) {
			drawOverlayColumn(b->fireRisk / 10, xOffset, yOffset, 1);
		}
	}
}

static void drawBuildingTopForDamageOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_ENGINEERS_POST) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->damageRisk > 0) {
		int draw = 1;
		if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_PIG_FARM) {
			draw = is_drawable_farm_corner(gridOffset, city_view_orientation());
		}
		if (draw) {
			drawOverlayColumn(b->damageRisk / 10, xOffset, yOffset, 1);
		}
	}
}

static void drawBuildingTopForCrimeOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_PREFECTURE) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->type == BUILDING_BURNING_RUIN) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize) {
		int happiness = b->sentiment.houseHappiness;
		if (happiness < 50) {
			int colVal;
			if (happiness <= 0) {
				colVal = 10;
			} else if (happiness <= 10) {
				colVal = 8;
			} else if (happiness <= 20) {
				colVal = 6;
			} else if (happiness <= 30) {
				colVal = 4;
			} else if (happiness <= 40) {
				colVal = 2;
			} else {
				colVal = 1;
			}
			drawOverlayColumn(colVal, xOffset, yOffset, 1);
		}
	}
}

static void drawBuildingTopForEntertainmentOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_THEATER:
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_ACTOR_COLONY:
		case BUILDING_GLADIATOR_SCHOOL:
		case BUILDING_LION_HOUSE:
		case BUILDING_CHARIOT_MAKER:
		case BUILDING_AMPHITHEATER:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_COLOSSEUM:
		case BUILDING_HIPPODROME:
			DRAWTOP_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.entertainment) {
				drawOverlayColumn(b->data.house.entertainment / 10, xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForEducationOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_ACADEMY:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_LIBRARY:
		case BUILDING_SCHOOL:
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.education) {
				drawOverlayColumn(b->data.house.education * 3 - 1,
					xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForTheaterOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_ACTOR_COLONY:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_THEATER:
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.theater) {
				drawOverlayColumn(b->data.house.theater / 10, xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForAmphitheaterOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_ACTOR_COLONY:
		case BUILDING_GLADIATOR_SCHOOL:
		case BUILDING_AMPHITHEATER:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.amphitheaterActor) {
				drawOverlayColumn(b->data.house.amphitheaterActor / 10, xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForColosseumOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_GLADIATOR_SCHOOL:
		case BUILDING_LION_HOUSE:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_COLOSSEUM:
			DRAWTOP_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.colosseumGladiator) {
				drawOverlayColumn(b->data.house.colosseumGladiator / 10, xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForHippodromeOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_HIPPODROME) {
		DRAWTOP_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->type == BUILDING_CHARIOT_MAKER) {
		DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.hippodrome) {
		drawOverlayColumn(b->data.house.hippodrome / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForFoodStocksOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_MARKET:
		case BUILDING_WHARF:
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_GRANARY:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize) {
				if (model_get_house(b->subtype.houseLevel)->food_types) {
					int pop = b->housePopulation;
					int stocks = 0;
					for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
						stocks += b->data.house.inventory[i];
					}
					int pctStocks = calc_percentage(stocks, pop);
					int colVal = 0;
					if (pctStocks <= 0) {
						colVal = 10;
					} else if (pctStocks < 100) {
						colVal = 5;
					} else if (pctStocks <= 200) {
						colVal = 1;
					}
					if (colVal) {
						drawOverlayColumn(colVal, xOffset, yOffset, 1);
					}
				}
			}
			break;
	}
}

static void drawBuildingTopForBathhouseOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_BATHHOUSE) {
		DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.bathhouse) {
		drawOverlayColumn(b->data.house.bathhouse / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForReligionOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_ORACLE:
		case BUILDING_SMALL_TEMPLE_CERES:
		case BUILDING_SMALL_TEMPLE_NEPTUNE:
		case BUILDING_SMALL_TEMPLE_MERCURY:
		case BUILDING_SMALL_TEMPLE_MARS:
		case BUILDING_SMALL_TEMPLE_VENUS:
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_LARGE_TEMPLE_CERES:
		case BUILDING_LARGE_TEMPLE_NEPTUNE:
		case BUILDING_LARGE_TEMPLE_MERCURY:
		case BUILDING_LARGE_TEMPLE_MARS:
		case BUILDING_LARGE_TEMPLE_VENUS:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.numGods) {
				drawOverlayColumn(b->data.house.numGods * 17 / 10, xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForSchoolOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_SCHOOL) {
		DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.school) {
		drawOverlayColumn(b->data.house.school / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForLibraryOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_LIBRARY) {
		DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.library) {
		drawOverlayColumn(b->data.house.library / 10,xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForAcademyOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_ACADEMY) {
		DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.academy) {
		drawOverlayColumn(b->data.house.academy / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForBarberOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_BARBER) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.barber) {
		drawOverlayColumn(b->data.house.barber / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForClinicsOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_DOCTOR) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.clinic) {
		drawOverlayColumn(b->data.house.clinic / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForHospitalOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_HOSPITAL) {
		DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.hospital) {
		drawOverlayColumn(b->data.house.hospital / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForTaxIncomeOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_SENATE_UPGRADED) {
		DRAWTOP_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->type == BUILDING_FORUM) {
		DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize) {
		int pct = calc_adjust_with_percentage(b->taxIncomeOrStorage / 2, Data_CityInfo.taxPercentage);
		if (pct > 0) {
			drawOverlayColumn(pct / 25, xOffset, yOffset, 0);
		}
	}
}

static int is_problem_cartpusher(int figure_id)
{
    if (figure_id) {
        figure *fig = figure_get(figure_id);
        return fig->actionState == FIGURE_ACTION_20_CARTPUSHER_INITIAL && fig->minMaxSeen;
    } else {
        return 0;
    }
}

static void drawBuildingTopForProblemsOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->houseSize) {
		return;
	}
	int type = b->type;
	if (type == BUILDING_FOUNTAIN || type == BUILDING_BATHHOUSE) {
		if (!b->hasWaterAccess) {
			b->showOnProblemOverlay = 1;
		}
	} else if (type >= BUILDING_WHEAT_FARM && type <= BUILDING_CLAY_PIT) {
		if (is_problem_cartpusher(b->figureId)) {
			b->showOnProblemOverlay = 1;
		}
	} else if (building_is_workshop(type)) {
		if (is_problem_cartpusher(b->figureId)) {
			b->showOnProblemOverlay = 1;
		} else if (b->loadsStored <= 0) {
			b->showOnProblemOverlay = 1;
		}
	}

	if (b->showOnProblemOverlay <= 0) {
		return;
	}

	if (type >= BUILDING_WHEAT_FARM && type <= BUILDING_PIG_FARM) {
		if (is_drawable_farmhouse(gridOffset, city_view_orientation())) {
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
		} else if (map_property_is_draw_tile(gridOffset)) {
            DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
		}
		return;
	}
	if (type == BUILDING_GRANARY) {
		const image *img = image_get(map_image_at(gridOffset));
		image_draw(image_group(GROUP_BUILDING_GRANARY) + 1,
			xOffset + img->sprite_offset_x,
			yOffset + img->sprite_offset_y - 30 -
			(img->height - 90));
		if (b->data.storage.resourceStored[RESOURCE_NONE] < 2400) {
			image_draw(image_group(GROUP_BUILDING_GRANARY) + 2,
				xOffset + 32, yOffset - 61);
			if (b->data.storage.resourceStored[RESOURCE_NONE] < 1800) {
				image_draw(image_group(GROUP_BUILDING_GRANARY) + 3,
					xOffset + 56, yOffset - 51);
			}
			if (b->data.storage.resourceStored[RESOURCE_NONE] < 1200) {
				image_draw(image_group(GROUP_BUILDING_GRANARY) + 4,
					xOffset + 91, yOffset - 51);
			}
			if (b->data.storage.resourceStored[RESOURCE_NONE] < 600) {
				image_draw(image_group(GROUP_BUILDING_GRANARY) + 5,
					xOffset + 118, yOffset - 61);
			}
		}
	}
	if (type == BUILDING_WAREHOUSE) {
		image_draw(image_group(GROUP_BUILDING_WAREHOUSE) + 17, xOffset - 4, yOffset - 42);
	}

	draw_top_with_size(gridOffset, xOffset, yOffset);
}

static void drawOverlayColumn(int height, int xOffset, int yOffset, int isRed)
{
	int graphicId = image_group(GROUP_OVERLAY_COLUMN);
	if (isRed) {
		graphicId += 9;
	}
	if (height > 10) {
		height = 10;
	}
	int capitalHeight = image_get(graphicId)->height;
	// draw base
	image_draw(graphicId + 2, xOffset + 9, yOffset - 8);
	if (height) {
		for (int i = 1; i < height; i++) {
			image_draw(graphicId + 1, xOffset + 17, yOffset - 8 - 10 * i + 13);
		}
		// top
		image_draw(graphicId,
			xOffset + 5, yOffset - 8 - capitalHeight - 10 * (height - 1) + 13);
	}
}
