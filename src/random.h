/*
 * random.h - Random number generator
 *
 * Copyright (C) 2012  Jon Lund Steffensen <jonlst@gmail.com>
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

#ifndef SRC_RANDOM_H_
#define SRC_RANDOM_H_

#include <string>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

class Random {
 protected:
  uint16_t state[3];

 public:
  Random();
  explicit Random(const uint16_t &value);
  Random(const Random &random_state);
  explicit Random(const std::string &string);
  Random(uint16_t base_0, uint16_t base_1, uint16_t base_2);

  uint16_t random();

  operator std::string() const;
  friend Random& operator^=(Random& left, const Random& right);
};

#endif  // SRC_RANDOM_H_
