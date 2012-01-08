/*
Minetest-c55
Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "mineral.h"
#include "gamedef.h"


const char *mineral_filenames[MINERAL_COUNT] =
{
	NULL,
	"mineral_coal.png",
	"mineral_iron.png"
};

std::string mineral_textures[MINERAL_COUNT];

void init_mineral()
{
	for(u32 i=0; i<MINERAL_COUNT; i++)
	{
		if(mineral_filenames[i] == NULL)
			continue;
		mineral_textures[i] = mineral_filenames[i];
	}
}

std::string mineral_block_texture(u8 mineral)
{
	if(mineral >= MINERAL_COUNT)
		return "";
	
	return mineral_textures[mineral];
}

InventoryItem getDiggedMineralItem(u8 mineral, IGameDef *gamedef)
{
	if(mineral == MINERAL_COAL)
		return InventoryItem("default:coal_lump", 1, 0, "", gamedef->idef());
	else if(mineral == MINERAL_IRON)
		return InventoryItem("default:iron_lump", 1, 0, "", gamedef->idef());
	else
		return InventoryItem();
}



