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

#include "collision.h"
#include "mapblock.h"
#include "map.h"
#include "nodedef.h"
#include "gamedef.h"

collisionMoveResult collisionMoveSimple(Map *map, IGameDef *gamedef,
		f32 pos_max_d, const core::aabbox3d<f32> &box_0,
		f32 stepheight, f32 dtime,
		v3f &pos_f, v3f &speed_f, v3f &accel_f)
{
	collisionMoveResult result;

	v3f oldpos_f = pos_f;
	v3s16 oldpos_i = floatToInt(oldpos_f, BS);

	/*
		Calculate new velocity
	*/
	speed_f += accel_f * dtime;

	/*
		Calculate new position
	*/
	pos_f += speed_f * dtime;

	/*
		Collision detection
	*/
	
	// position in nodes
	v3s16 pos_i = floatToInt(pos_f, BS);
	
	/*
		Collision uncertainty radius
		Make it a bit larger than the maximum distance of movement
	*/
	f32 d = pos_max_d * 1.1;
	// A fairly large value in here makes moving smoother
	//f32 d = 0.15*BS;

	// This should always apply, otherwise there are glitches
	assert(d > pos_max_d);
	
	/*
		Calculate collision box
	*/
	core::aabbox3d<f32> box = box_0;
	box.MaxEdge += pos_f;
	box.MinEdge += pos_f;
	core::aabbox3d<f32> oldbox = box_0;
	oldbox.MaxEdge += oldpos_f;
	oldbox.MinEdge += oldpos_f;

	/*
		Go through every node around the object
	*/
	s16 min_x = (box_0.MinEdge.X / BS) - 2;
	s16 min_y = (box_0.MinEdge.Y / BS) - 2;
	s16 min_z = (box_0.MinEdge.Z / BS) - 2;
	s16 max_x = (box_0.MaxEdge.X / BS) + 1;
	s16 max_y = (box_0.MaxEdge.Y / BS) + 1;
	s16 max_z = (box_0.MaxEdge.Z / BS) + 1;
	for(s16 y = oldpos_i.Y + max_y; y >= oldpos_i.Y + min_y; y--)
	for(s16 z = oldpos_i.Z + min_z; z <= oldpos_i.Z + max_z; z++)
	for(s16 x = oldpos_i.X + min_x; x <= oldpos_i.X + max_x; x++)
	{
		bool is_unloaded = false;
		try{
			// Object collides into walkable nodes
			MapNode n = map->getNode(v3s16(x,y,z));
			if(gamedef->getNodeDefManager()->get(n).walkable == false)
				continue;
		}
		catch(InvalidPositionException &e)
		{
			is_unloaded = true;
			// Doing nothing here will block the object from
			// walking over map borders
		}

		core::aabbox3d<f32> nodebox = getNodeBox(v3s16(x,y,z), BS);
		
		/*
			See if the object is touching ground.

			Object touches ground if object's minimum Y is near node's
			maximum Y and object's X-Z-area overlaps with the node's
			X-Z-area.

			Use 0.15*BS so that it is easier to get on a node.
		*/
		if(
				//fabs(nodebox.MaxEdge.Y-box.MinEdge.Y) < d
				fabs(nodebox.MaxEdge.Y-box.MinEdge.Y) < 0.15*BS
				&& nodebox.MaxEdge.X-d > box.MinEdge.X
				&& nodebox.MinEdge.X+d < box.MaxEdge.X
				&& nodebox.MaxEdge.Z-d > box.MinEdge.Z
				&& nodebox.MinEdge.Z+d < box.MaxEdge.Z
		){
			result.touching_ground = true;
			if(is_unloaded)
				result.standing_on_unloaded = true;
		}
		
		// If object doesn't intersect with node, ignore node.
		if(box.intersectsWithBox(nodebox) == false)
			continue;
		
		int walking_up_step = 0; // 0 = no, 1 = yes, 2 = inhibited

		/*
			Go through every axis
		*/
		v3f dirs[3] = {
			v3f(0,0,1), // back-front
			v3f(0,1,0), // top-bottom
			v3f(1,0,0), // right-left
		};
		for(u16 i=0; i<3; i++)
		{
			/*
				Calculate values along the axis
			*/
			f32 nodemax = nodebox.MaxEdge.dotProduct(dirs[i]);
			f32 nodemin = nodebox.MinEdge.dotProduct(dirs[i]);
			f32 objectmax = box.MaxEdge.dotProduct(dirs[i]);
			f32 objectmin = box.MinEdge.dotProduct(dirs[i]);
			f32 objectmax_old = oldbox.MaxEdge.dotProduct(dirs[i]);
			f32 objectmin_old = oldbox.MinEdge.dotProduct(dirs[i]);
			
			/*
				Check collision for the axis.
				Collision happens when object is going through a surface.
			*/
			bool negative_axis_collides =
				(nodemax > objectmin && nodemax <= objectmin_old + d
					&& speed_f.dotProduct(dirs[i]) < 0);
			bool positive_axis_collides =
				(nodemin < objectmax && nodemin >= objectmax_old - d
					&& speed_f.dotProduct(dirs[i]) > 0);
			bool main_axis_collides =
					negative_axis_collides || positive_axis_collides;
			
			if(!main_axis_collides)
				continue;

			/*
				Check overlap of object and node in other axes
			*/
			bool other_axes_overlap = true;
			bool other_axes_overlap_plus_step = true;
			for(u16 j=0; j<3; j++)
			{
				if(j == i)
					continue;
				f32 nodemax = nodebox.MaxEdge.dotProduct(dirs[j]);
				f32 nodemin = nodebox.MinEdge.dotProduct(dirs[j]);
				f32 objectmax = box.MaxEdge.dotProduct(dirs[j]);
				f32 objectmin = box.MinEdge.dotProduct(dirs[j]);
				f32 objectmax_old = oldbox.MaxEdge.dotProduct(dirs[j]);
				f32 objectmin_old = oldbox.MinEdge.dotProduct(dirs[j]);
				f32 objectmax_all = MYMAX(objectmax, objectmax_old);
				f32 objectmin_all = MYMIN(objectmin, objectmin_old);

				if(nodemax <= objectmin_all || nodemin >= objectmax_all)
					other_axes_overlap = false;

				if(j == 1 && (nodemax <= objectmin_all + stepheight || nodemin >= objectmax_all))
					other_axes_overlap_plus_step = false;
			}
			
			/*
				If this is a collision, revert the pos_f in the main
				direction.
			*/
			if(other_axes_overlap_plus_step == false && other_axes_overlap)
			{
				// Special case: walking up a step
				if(walking_up_step == 0)
					walking_up_step = 1;
			}
			else if(other_axes_overlap)
			{
				// Collision occurred
				speed_f -= speed_f.dotProduct(dirs[i]) * dirs[i];
				pos_f -= pos_f.dotProduct(dirs[i]) * dirs[i];
				pos_f += oldpos_f.dotProduct(dirs[i]) * dirs[i];
				result.collides = true;
				if(i != 1)
					result.collides_xz = true;
				walking_up_step = 2;  // inhibit
			}
		
		}

		if(walking_up_step == 1)
		{
			dstream << "walking up step\n";
			speed_f.Y = 0;

			f32 ydiff = nodebox.MaxEdge.Y - box.MinEdge.Y;
			pos_f.Y += ydiff;
			box.MinEdge.Y += ydiff;
			box.MaxEdge.Y += ydiff;
		}
	} // xyz
	
	return result;
}

collisionMoveResult collisionMovePrecise(Map *map, IGameDef *gamedef,
		f32 pos_max_d, const core::aabbox3d<f32> &box_0,
		f32 stepheight, f32 dtime,
		v3f &pos_f, v3f &speed_f, v3f &accel_f)
{
	collisionMoveResult final_result;

	// Don't allow overly huge dtime
	if(dtime > 2.0)
		dtime = 2.0;
	
	f32 dtime_downcount = dtime;

	u32 loopcount = 0;
	do
	{
		loopcount++;

		// Maximum time increment (for collision detection etc)
		// time = distance / speed
		f32 dtime_max_increment = 1.0;
		if(speed_f.getLength() != 0)
			dtime_max_increment = pos_max_d / speed_f.getLength();

		// Maximum time increment is 10ms or lower
		if(dtime_max_increment > 0.01)
			dtime_max_increment = 0.01;

		f32 dtime_part;
		if(dtime_downcount > dtime_max_increment)
		{
			dtime_part = dtime_max_increment;
			dtime_downcount -= dtime_part;
		}
		else
		{
			dtime_part = dtime_downcount;
			/*
				Setting this to 0 (no -=dtime_part) disables an infinite loop
				when dtime_part is so small that dtime_downcount -= dtime_part
				does nothing
			*/
			dtime_downcount = 0;
		}

		collisionMoveResult result = collisionMoveSimple(map, gamedef,
				pos_max_d, box_0, stepheight, dtime_part,
				pos_f, speed_f, accel_f);

		if(result.touching_ground)
			final_result.touching_ground = true;
		if(result.collides)
			final_result.collides = true;
		if(result.collides_xz)
			final_result.collides_xz = true;
		if(result.standing_on_unloaded)
			final_result.standing_on_unloaded = true;
	}
	while(dtime_downcount > 0.001);
		

	return final_result;
}
