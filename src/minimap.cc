/*
 * minimap.cc - Minimap GUI component
 *
 * Copyright (C) 2013-2014  Jon Lund Steffensen <jonlst@gmail.com>
 *
 * This file is part of freeserf.
 *
 * freeserf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * freeserf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freeserf.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/minimap.h"

#include <algorithm>

#include "src/misc.h"
#include "src/game.h"
#include "src/data.h"
#include "src/gfx.h"
#include "src/interface.h"
#include "src/viewport.h"

#define MINIMAP_MAX_SCALE  8

void
Minimap::draw_minimap_point(int col, int row, uint8_t color, int density) {
  int map_width = map->get_cols() * scale;
  int map_height = map->get_rows() * scale;

  if (0 == map_width || 0 == map_height) {
    return;
  }

  int mm_y = row*scale - offset_y;
  col -= (map->get_rows()/2) * static_cast<int>(mm_y / map_height);
  mm_y = mm_y % map_height;

  while (mm_y < height) {
    if (mm_y >= -density) {
      int mm_x = col*scale - (row*scale)/2 - offset_x;
      mm_x = mm_x % map_width;
      while (mm_x < width) {
        if (mm_x >= -density) {
          frame->fill_rect(mm_x, mm_y, density, density, color);
        }
        mm_x += map_width;
      }
    }
    col -= map->get_rows()/2;
    mm_y += map_height;
  }
}

void
Minimap::draw_minimap_map() {
  uint8_t *color_data = map->get_minimap();
  for (unsigned int row = 0; row < map->get_rows(); row++) {
    for (unsigned int col = 0; col < map->get_cols(); col++) {
      uint8_t color = *(color_data++);
      draw_minimap_point(col, row, color, scale);
    }
  }
}

void
MinimapGame::draw_minimap_ownership(int density) {
  for (unsigned int row = 0; row < map->get_rows(); row++) {
    for (unsigned int col = 0; col < map->get_cols(); col++) {
      MapPos pos = map->pos(col, row);
      if (map->has_owner(pos)) {
        int color =
            interface->get_game()->get_player(map->get_owner(pos))->get_color();
        draw_minimap_point(col, row, color, density);
      }
    }
  }
}

void
MinimapGame::draw_minimap_roads() {
  for (unsigned int row = 0; row < map->get_rows(); row++) {
    for (unsigned int col = 0; col < map->get_cols(); col++) {
      int pos = map->pos(col, row);
      if (map->paths(pos)) {
        draw_minimap_point(col, row, 1, scale);
      }
    }
  }
}

void
MinimapGame::draw_minimap_buildings() {
  const int building_remap[] = {
    Building::TypeCastle,
    Building::TypeStock, Building::TypeTower, Building::TypeHut,
    Building::TypeFortress, Building::TypeToolMaker, Building::TypeSawmill,
    Building::TypeWeaponSmith, Building::TypeStonecutter,
    Building::TypeBoatbuilder, Building::TypeForester, Building::TypeLumberjack,
    Building::TypePigFarm, Building::TypeFarm, Building::TypeFisher,
    Building::TypeMill, Building::TypeButcher, Building::TypeBaker,
    Building::TypeStoneMine, Building::TypeCoalMine, Building::TypeIronMine,
    Building::TypeGoldMine, Building::TypeSteelSmelter,
    Building::TypeGoldSmelter
  };

  for (unsigned int row = 0; row < map->get_rows(); row++) {
    for (unsigned int col = 0; col < map->get_cols(); col++) {
      int pos = map->pos(col, row);
      int obj = map->get_obj(pos);
      if (obj > Map::ObjectFlag && obj <= Map::ObjectCastle) {
        int color =
            interface->get_game()->get_player(map->get_owner(pos))->get_color();
        if (advanced > 0) {
          Building *bld = interface->get_game()->get_building_at_pos(pos);
          if (bld->get_type() == building_remap[advanced]) {
            draw_minimap_point(col, row, color, scale);
          }
        } else {
          draw_minimap_point(col, row, color, scale);
        }
      }
    }
  }
}

void
MinimapGame::draw_minimap_traffic() {
  for (unsigned int row = 0; row < map->get_rows(); row++) {
    for (unsigned int col = 0; col < map->get_cols(); col++) {
      int pos = map->pos(col, row);
      if (map->get_idle_serf(pos)) {
        int color =
            interface->get_game()->get_player(map->get_owner(pos))->get_color();
        draw_minimap_point(col, row, color, scale);
      }
    }
  }
}

void
Minimap::draw_minimap_grid() {
  for (unsigned int y = 0; y < map->get_rows() * scale; y += 2) {
    draw_minimap_point(0, y, 47, 1);
    draw_minimap_point(0, y+1, 1, 1);
  }

  for (unsigned int x = 0; x < map->get_cols() * scale; x += 2) {
    draw_minimap_point(x, 0, 47, 1);
    draw_minimap_point(x+1, 0, 1, 1);
  }
}

void
Minimap::draw_minimap_rect() {
  int y = height/2;
  int x = width/2;
  frame->draw_transp_sprite(x, y, 354, true);
}

void
Minimap::internal_draw() {
  if (map == NULL) {
    frame->fill_rect(0, 0, width, height, 1);
    return;
  }

  draw_minimap_map();
  draw_minimap_grid();
}

int
Minimap::handle_scroll(int up) {
  int scale_ = 0;
  if (up) {
    scale_ = scale + 1;
  } else {
    scale_ = scale - 1;
  }

  set_scale(clamp(1, scale_, MINIMAP_MAX_SCALE));
  return 0;
}

bool
Minimap::handle_drag(int dx, int dy) {
  if (dx != 0 || dy != 0) {
    move_by_pixels(dx, dy);
  }

  return true;
}

Minimap::Minimap(Map *map) {
  offset_x = 0;
  offset_y = 0;
  scale = 1;

  advanced = -1;
  flags = 8;

  this->map = map;
}

void
Minimap::set_map(Map *map) {
  this->map = map;

  set_redraw();
}

/* Set the scale of the map (zoom). Must be positive. */
void
Minimap::set_scale(int scale) {
  MapPos pos = get_current_map_pos();
  this->scale = scale;
  move_to_map_pos(pos);

  set_redraw();
}

void
Minimap::screen_pix_from_map_pix(int mx, int my, int *sx, int *sy) {
  int width = map->get_cols() * scale;
  int height = map->get_rows() * scale;

  *sx = mx - offset_x;
  *sy = my - offset_y;

  while (*sy < 0) {
    *sx -= height/2;
    *sy += height;
  }

  while (*sy >= height) {
    *sx += height/2;
    *sy -= height;
  }

  while (*sx < 0) *sx += width;
  while (*sx >= width) *sx -= width;
}

void
Minimap::map_pix_from_map_coord(MapPos pos, int *mx, int *my) {
  int width = map->get_cols() * scale;
  int height = map->get_rows() * scale;

  *mx = scale * map->pos_col(pos) - (scale * map->pos_row(pos))/2;
  *my = scale * map->pos_row(pos);

  if (*my < 0) {
    *mx -= height/2;
    *my += height;
  }

  if (*mx < 0) *mx += width;
  else if (*mx >= width) *mx -= width;
}

MapPos
Minimap::map_pos_from_screen_pix(int x, int y) {
  int mx = x + offset_x;
  int my = y + offset_y;

  int col = ((my/2 + mx)/scale) & map->get_col_mask();
  int row = (my/scale) & map->get_row_mask();

  return map->pos(col, row);
}

MapPos
Minimap::get_current_map_pos() {
  return map_pos_from_screen_pix(width/2, height/2);
}

void
Minimap::move_to_map_pos(MapPos pos) {
  int mx, my;
  map_pix_from_map_coord(pos, &mx, &my);

  int map_width = map->get_cols()*scale;
  int map_height = map->get_rows()*scale;

  /* Center view */
  mx -= width/2;
  my -= height/2;

  if (my < 0) {
    mx -= map_height/2;
    my += map_height;
  }

  if (mx < 0) mx += map_width;
  else if (mx >= map_width) mx -= map_width;

  offset_x = mx;
  offset_y = my;

  set_redraw();
}

void
Minimap::move_by_pixels(int dx, int dy) {
  int width = map->get_cols() * scale;
  int height = map->get_rows() * scale;

  offset_x += dx;
  offset_y += dy;

  if (offset_y < 0) {
    offset_y += height;
    offset_x -= height/2;
  } else if (offset_y >= height) {
    offset_y -= height;
    offset_x += height/2;
  }

  if (offset_x >= width) offset_x -= width;
  else if (offset_x < 0) offset_x += width;

  set_redraw();
}

MinimapGame::MinimapGame(Interface *interface, Game *game)
  : Minimap(game->get_map()) {
  this->interface = interface;
  this->game = game;
}

void
MinimapGame::internal_draw() {
  if (BIT_TEST(flags, 1)) {
    frame->fill_rect(0, 0, 128, 128, 1);
    draw_minimap_ownership(2);
  } else {
    draw_minimap_map();
    if (BIT_TEST(flags, 0)) {
      draw_minimap_ownership(1);
    }
  }

  if (BIT_TEST(flags, 2)) {
    draw_minimap_roads();
  }

  if (BIT_TEST(flags, 3)) {
    draw_minimap_buildings();
  }

  if (BIT_TEST(flags, 4)) {
    draw_minimap_grid();
  }

  if (advanced > 0) {
    draw_minimap_traffic();
  }

  draw_minimap_rect();
}

bool
MinimapGame::handle_click_left(int x, int y) {
  MapPos pos = map_pos_from_screen_pix(x, y);
  interface->get_viewport()->move_to_map_pos(pos);

  interface->update_map_cursor_pos(pos);
  interface->close_popup();

  return true;
}
