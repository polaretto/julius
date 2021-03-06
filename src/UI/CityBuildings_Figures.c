#include "CityBuildings.h"
#include "CityBuildings_private.h"

#include "building/building.h"
#include "city/view.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "game/resource.h"
#include "game/state.h"

static building *get_entertainment_building(const figure *f)
{
    if (f->actionState == FIGURE_ACTION_94_ENTERTAINER_ROAMING ||
        f->actionState == FIGURE_ACTION_95_ENTERTAINER_RETURNING) {
        return building_get(f->buildingId);
    } else {
        return building_get(f->destinationBuildingId);
    }
}

static int showOnOverlay(figure *f)
{
	switch (game_state_overlay()) {
		case OVERLAY_WATER:
		case OVERLAY_DESIRABILITY:
			return 0;
		case OVERLAY_NATIVE:
			return f->type == FIGURE_INDIGENOUS_NATIVE || f->type == FIGURE_MISSIONARY;
		case OVERLAY_FIRE:
			return f->type == FIGURE_PREFECT;
		case OVERLAY_DAMAGE:
			return f->type == FIGURE_ENGINEER;
		case OVERLAY_TAX_INCOME:
			return f->type == FIGURE_TAX_COLLECTOR;
		case OVERLAY_CRIME:
			return f->type == FIGURE_PREFECT || f->type == FIGURE_PROTESTER ||
				f->type == FIGURE_CRIMINAL || f->type == FIGURE_RIOTER;
		case OVERLAY_ENTERTAINMENT:
			return f->type == FIGURE_ACTOR || f->type == FIGURE_GLADIATOR ||
				f->type == FIGURE_LION_TAMER || f->type == FIGURE_CHARIOTEER;
		case OVERLAY_EDUCATION:
			return f->type == FIGURE_SCHOOL_CHILD || f->type == FIGURE_LIBRARIAN ||
				f->type == FIGURE_TEACHER;
		case OVERLAY_THEATER:
			if (f->type == FIGURE_ACTOR) {
				return get_entertainment_building(f)->type == BUILDING_THEATER;
			}
			return 0;
		case OVERLAY_AMPHITHEATER:
			if (f->type == FIGURE_ACTOR || f->type == FIGURE_GLADIATOR) {
				return get_entertainment_building(f)->type == BUILDING_AMPHITHEATER;
			}
			return 0;
		case OVERLAY_COLOSSEUM:
			if (f->type == FIGURE_GLADIATOR) {
				return get_entertainment_building(f)->type == BUILDING_COLOSSEUM;
			} else if (f->type == FIGURE_LION_TAMER) {
				return 1;
			}
			return 0;
		case OVERLAY_HIPPODROME:
			return f->type == FIGURE_CHARIOTEER;
		case OVERLAY_RELIGION:
			return f->type == FIGURE_PRIEST;
		case OVERLAY_SCHOOL:
			return f->type == FIGURE_SCHOOL_CHILD;
		case OVERLAY_LIBRARY:
			return f->type == FIGURE_LIBRARIAN;
		case OVERLAY_ACADEMY:
			return f->type == FIGURE_TEACHER;
		case OVERLAY_BARBER:
			return f->type == FIGURE_BARBER;
		case OVERLAY_BATHHOUSE:
			return f->type == FIGURE_BATHHOUSE_WORKER;
		case OVERLAY_CLINIC:
			return f->type == FIGURE_DOCTOR;
		case OVERLAY_HOSPITAL:
			return f->type == FIGURE_SURGEON;
		case OVERLAY_FOOD_STOCKS:
			if (f->type == FIGURE_MARKET_BUYER || f->type == FIGURE_MARKET_TRADER ||
				f->type == FIGURE_DELIVERY_BOY || f->type == FIGURE_FISHING_BOAT) {
				return 1;
			} else if (f->type == FIGURE_CART_PUSHER) {
				return resource_is_food(f->resourceId);
			}
			return 0;
		case OVERLAY_PROBLEMS:
			if (f->type == FIGURE_LABOR_SEEKER) {
				return building_get(f->buildingId)->showOnProblemOverlay;
			} else if (f->type == FIGURE_CART_PUSHER) {
				return f->actionState == FIGURE_ACTION_20_CARTPUSHER_INITIAL || f->minMaxSeen;
			}
			return 0;
	}
	return 1;
}

static void drawFigureWithCart(figure *f, int xOffset, int yOffset)
{
	if (f->yOffsetCart >= 0) {
		image_draw(f->graphicId, xOffset, yOffset);
		image_draw(f->cartGraphicId, xOffset + f->xOffsetCart, yOffset + f->yOffsetCart);
	} else {
		image_draw(f->cartGraphicId, xOffset + f->xOffsetCart, yOffset + f->yOffsetCart);
		image_draw(f->graphicId, xOffset, yOffset);
	}
}

static void drawHippodromeHorses(figure *f, int xOffset, int yOffset)
{
	int val = f->waitTicksMissile;
	switch (city_view_orientation()) {
		case DIR_0_TOP:
			xOffset += 10;
			if (val <= 10) {
				yOffset -= 2;
			} else if (val <= 11) {
				yOffset -= 10;
			} else if (val <= 12) {
				yOffset -= 18;
			} else if (val <= 13) {
				yOffset -= 16;
			} else if (val <= 20) {
				yOffset -= 14;
			} else if (val <= 21) {
				yOffset -= 10;
			} else {
				yOffset -= 2;
			}
			break;
		case DIR_2_RIGHT:
			xOffset -= 10;
			if (val <= 9) {
				yOffset -= 12;
			} else if (val <= 10) {
				yOffset += 4;
			} else if (val <= 11) {
				xOffset -= 5;
				yOffset += 2;
			} else if (val <= 13) {
				xOffset -= 5;
			} else if (val <= 20) {
				yOffset -= 2;
			} else if (val <= 21) {
				yOffset -= 6;
			} else {
				yOffset -= 12;
			}
		case DIR_4_BOTTOM:
			xOffset += 20;
			if (val <= 9) {
				yOffset += 4;
			} else if (val <= 10) {
				xOffset += 10;
				yOffset += 4;
			} else if (val <= 11) {
				xOffset += 10;
				yOffset -= 4;
			} else if (val <= 13) {
				yOffset -= 6;
			} else if (val <= 20) {
				yOffset -= 12;
			} else if (val <= 21) {
				yOffset -= 10;
			} else {
				yOffset -= 2;
			}
			break;
		case DIR_6_LEFT:
			xOffset -= 10;
			if (val <= 9) {
				yOffset -= 12;
			} else if (val <= 10) {
				yOffset += 4;
			} else if (val <= 11) {
				yOffset += 2;
			} else if (val <= 13) {
				// no change
			} else if (val <= 20) {
				yOffset -= 2;
			} else if (val <= 21) {
				yOffset -= 6;
			} else {
				yOffset -= 12;
			}
			break;
	}
	drawFigureWithCart(f, xOffset, yOffset);
}

static int tileOffsetToPixelOffsetX(int x, int y)
{
	int dir = city_view_orientation();
	if (dir == DIR_0_TOP || dir == DIR_4_BOTTOM) {
		int base = 2 * x - 2 * y;
		return dir == DIR_0_TOP ? base : -base;
	} else {
		int base = 2 * x + 2 * y;
		return dir == DIR_2_RIGHT ? base : -base;
	}
}

static int tileOffsetToPixelOffsetY(int x, int y)
{
	int dir = city_view_orientation();
	if (dir == DIR_0_TOP || dir == DIR_4_BOTTOM) {
		int base = x + y;
		return dir == DIR_0_TOP ? base : -base;
	} else {
		int base = x - y;
		return dir == DIR_6_LEFT ? base : -base;
	}
}

static int tileProgressToPixelOffsetX(int direction, int progress)
{
	int offset = 0;
	if (direction == 0 || direction == 2) {
		offset = 2 * progress - 28;
	} else if (direction == 1) {
		offset = 4 * progress - 56;
	} else if (direction == 3 || direction == 7) {
		offset = 0;
	} else if (direction == 4 || direction == 6) {
		offset = 28 - 2 * progress;
	} else if (direction == 5) {
		offset = 56 - 4 * progress;
	}
	return offset;
}

static int tileProgressToPixelOffsetY(int direction, int progress)
{
	int offset = 0;
	if (direction == 0 || direction == 6) {
		offset = 14 - progress;
	} else if (direction == 1 || direction == 5) {
		offset = 0;
	} else if (direction == 2 || direction == 4) {
		offset = progress - 14;
	} else if (direction == 3) {
		offset = 2 * progress - 28;
	} else if (direction == 7) {
		offset = 28 - 2 * progress;
	}
	return offset;
}

void UI_CityBuildings_drawFigure(int figureId, int xOffset, int yOffset, int selectedFigureId, struct UI_CityPixelCoordinate *coord)
{
	figure *f = figure_get(figureId);

	// determining x/y offset on tile
	int xTileOffset = 0;
	int yTileOffset = 0;
	if (f->useCrossCountry) {
		xTileOffset = tileOffsetToPixelOffsetX(f->crossCountryX % 15, f->crossCountryY % 15);
		yTileOffset = tileOffsetToPixelOffsetY(f->crossCountryX % 15, f->crossCountryY % 15);
		yTileOffset -= f->missileDamage;
	} else {
		int direction = (8 + f->direction - city_view_orientation()) % 8;
		xTileOffset = tileProgressToPixelOffsetX(direction, f->progressOnTile);
		yTileOffset = tileProgressToPixelOffsetY(direction, f->progressOnTile);
		yTileOffset -= f->currentHeight;
		if (f->numPreviousFiguresOnSameTile && f->type != FIGURE_BALLISTA) {
			static const int xOffsets[] = {
				0, 8, 8, -8, -8, 0, 16, 0, -16, 8, -8, 16, -16, 16, -16, 8, -8, 0, 24, 0, -24, 0, 0, 0
			};
			static const int yOffsets[] = {
				0, 0, 8, 8, -8, -16, 0, 16, 0, -16, 16, 8, -8, -8, 8, 16, -16, -24, 0, 24, 0, 0, 0, 0
			};
			xTileOffset += xOffsets[f->numPreviousFiguresOnSameTile];
			yTileOffset += yOffsets[f->numPreviousFiguresOnSameTile];
		}
	}

	xTileOffset += 29;
	yTileOffset += 15;

	const image *img = f->isEnemyGraphic ? image_get_enemy(f->graphicId) : image_get(f->graphicId);
	xOffset += xTileOffset - img->sprite_offset_x;
	yOffset += yTileOffset - img->sprite_offset_y;

	// excluding figures
	if (selectedFigureId == 9999) {
		if (!showOnOverlay(f)) {
			return;
		}
	} else if (selectedFigureId) {
		if (figureId != selectedFigureId) {
			return;
		}
		if (coord) {
			coord->x = xOffset;
			coord->y = yOffset;
		}
	}

	// actual drawing
	if (f->cartGraphicId) {
		switch (f->type) {
			case FIGURE_CART_PUSHER:
			case FIGURE_WAREHOUSEMAN:
			case FIGURE_LION_TAMER:
			case FIGURE_DOCKER:
			case FIGURE_NATIVE_TRADER:
			case FIGURE_IMMIGRANT:
			case FIGURE_EMIGRANT:
				drawFigureWithCart(f, xOffset, yOffset);
				break;
			case FIGURE_HIPPODROME_HORSES:
				drawHippodromeHorses(f, xOffset, yOffset);
				break;
			case FIGURE_FORT_STANDARD:
				if (!formation_get(f->formationId)->in_distant_battle) {
					// base
					image_draw(f->graphicId, xOffset, yOffset);
					// flag
					image_draw(f->cartGraphicId,
						xOffset, yOffset - image_get(f->cartGraphicId)->height);
					// top icon
					int iconGraphicId = image_group(GROUP_FIGURE_FORT_STANDARD_ICONS) + f->formationId - 1;
					image_draw(iconGraphicId,
						xOffset, yOffset - image_get(iconGraphicId)->height - image_get(f->cartGraphicId)->height);
				}
				break;
			default:
				image_draw(f->graphicId, xOffset, yOffset);
				break;
		}
	} else {
		if (f->isEnemyGraphic) {
			image_draw_enemy(f->graphicId, xOffset, yOffset);
		} else {
			image_draw(f->graphicId, xOffset, yOffset);
		}
	}
}
