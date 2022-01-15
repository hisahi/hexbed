/****************************************************************************/
/*                                                                          */
/* HexBed -- Hex editor                                                     */
/* Copyright (c) 2021-2022 Sampo Hippeläinen (hisahi)                       */
/*                                                                          */
/* This program is free software: you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or        */
/* (at your option) any later version.                                      */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>.   */
/*                                                                          */
/****************************************************************************/
// common/random.cc -- impl for randomization utilities

#include "common/random.hh"

#include <random>

namespace hexbed {

static std::random_device randdev;
static std::default_random_engine randeng(randdev());
static std::uniform_int_distribution<byte> randbyte(0, 255);

void randomizeBuffer(RandomType type, byte* buffer, std::size_t size) {
    switch (type) {
    case RandomType::Fast:
        for (std::size_t i = 0; i < size; ++i) buffer[i] = randbyte(randdev);
        break;
    case RandomType::Good:
        for (std::size_t i = 0; i < size; ++i) buffer[i] = randbyte(randeng);
        break;
    }
}

};  // namespace hexbed
