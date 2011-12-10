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
#include "log.h"
#include <algorithm>  // std::sort
#include <vector>

// Helper function:
// Checks for collision of a moving aabbox with a static aabbox
// Returns -1 if no collision, 0 if X collision, 1 if Y collision, 2 if Z collision
// The time after which the collision occurs is stored in dtime.
int axisAlignedCollision(
		const aabb3f &staticbox, const aabb3f &movingbox,
		const v3f &speed, f32 d, f32 &dtime)
{
	//TimeTaker tt("axisAlignedCollision");

	f32 xsize = (staticbox.MaxEdge.X - staticbox.MinEdge.X);
	f32 ysize = (staticbox.MaxEdge.Y - staticbox.MinEdge.Y);
	f32 zsize = (staticbox.MaxEdge.Z - staticbox.MinEdge.Z);

	aabb3f relbox(
			movingbox.MinEdge.X - staticbox.MinEdge.X,
			movingbox.MinEdge.Y - staticbox.MinEdge.Y,
			movingbox.MinEdge.Z - staticbox.MinEdge.Z,
			movingbox.MaxEdge.X - staticbox.MinEdge.X,
			movingbox.MaxEdge.Y - staticbox.MinEdge.Y,
			movingbox.MaxEdge.Z - staticbox.MinEdge.Z
	);

	if(speed.X > 0) // Check for collision with X- plane
	{
		if(relbox.MaxEdge.X <= d)
		{
			dtime = - relbox.MaxEdge.X / speed.X;
			if((relbox.MinEdge.Y + speed.Y * dtime < ysize) &&
					(relbox.MaxEdge.Y + speed.Y * dtime > 0) &&
					(relbox.MinEdge.Z + speed.Z * dtime < zsize) &&
					(relbox.MaxEdge.Z + speed.Z * dtime > 0))
				return 0;
		}
		else if(relbox.MinEdge.X > xsize)
		{
			return -1;
		}
	}
	else if(speed.X < 0) // Check for collision with X+ plane
	{
		if(relbox.MinEdge.X >= xsize - d)
		{
			dtime = (xsize - relbox.MinEdge.X) / speed.X;
			if((relbox.MinEdge.Y + speed.Y * dtime < ysize) &&
					(relbox.MaxEdge.Y + speed.Y * dtime > 0) &&
					(relbox.MinEdge.Z + speed.Z * dtime < zsize) &&
					(relbox.MaxEdge.Z + speed.Z * dtime > 0))
				return 0;
		}
		else if(relbox.MaxEdge.X < 0)
		{
			return -1;
		}
	}

	// NO else if here

	if(speed.Y > 0) // Check for collision with Y- plane
	{
		if(relbox.MaxEdge.Y <= d)
		{
			dtime = - relbox.MaxEdge.Y / speed.Y;
			if((relbox.MinEdge.X + speed.X * dtime < xsize) &&
					(relbox.MaxEdge.X + speed.X * dtime > 0) &&
					(relbox.MinEdge.Z + speed.Z * dtime < zsize) &&
					(relbox.MaxEdge.Z + speed.Z * dtime > 0))
				return 1;
		}
		else if(relbox.MinEdge.Y > ysize)
		{
			return -1;
		}
	}
	else if(speed.Y < 0) // Check for collision with Y+ plane
	{
		if(relbox.MinEdge.Y >= ysize - d)
		{
			dtime = (ysize - relbox.MinEdge.Y) / speed.Y;
			if((relbox.MinEdge.X + speed.X * dtime < xsize) &&
					(relbox.MaxEdge.X + speed.X * dtime > 0) &&
					(relbox.MinEdge.Z + speed.Z * dtime < zsize) &&
					(relbox.MaxEdge.Z + speed.Z * dtime > 0))
				return 1;
		}
		else if(relbox.MaxEdge.Y < 0)
		{
			return -1;
		}
	}

	// NO else if here

	if(speed.Z > 0) // Check for collision with Z- plane
	{
		if(relbox.MaxEdge.Z <= d)
		{
			dtime = - relbox.MaxEdge.Z / speed.Z;
			if((relbox.MinEdge.X + speed.X * dtime < xsize) &&
					(relbox.MaxEdge.X + speed.X * dtime > 0) &&
					(relbox.MinEdge.Y + speed.Y * dtime < ysize) &&
					(relbox.MaxEdge.Y + speed.Y * dtime > 0))
				return 2;
		}
		//else if(relbox.MinEdge.Z > zsize)
		//{
		//	return -1;
		//}
	}
	else if(speed.Z < 0) // Check for collision with Z+ plane
	{
		if(relbox.MinEdge.Z >= zsize - d)
		{
			dtime = (zsize - relbox.MinEdge.Z) / speed.Z;
			if((relbox.MinEdge.X + speed.X * dtime < xsize) &&
					(relbox.MaxEdge.X + speed.X * dtime > 0) &&
					(relbox.MinEdge.Y + speed.Y * dtime < ysize) &&
					(relbox.MaxEdge.Y + speed.Y * dtime > 0))
				return 2;
		}
		//else if(relbox.MaxEdge.Z < 0)
		//{
		//	return -1;
		//}
	}

	return -1;
}


struct Nodebox
{
	aabb3f box;
	bool is_unloaded;
	bool is_step_up;

	Nodebox(const aabb3f &box_, bool is_unloaded_, bool is_step_up_):
		box(box_), is_unloaded(is_unloaded_), is_step_up(is_step_up_) {}
};

struct NodeboxCompareByDistance
{
	v3f origin;

	NodeboxCompareByDistance(v3f origin_): origin(origin_) {}

	// Computes the manhattan distance between the box and m_origin.
	// Returns 0 if m_origin is inside the box.
	inline f32 getDistance(const Nodebox &nb) const
	{
		f32 xd = getDistanceFromInterval(origin.X, nb.box.MinEdge.X, nb.box.MaxEdge.X);
		f32 yd = getDistanceFromInterval(origin.Y, nb.box.MinEdge.Y, nb.box.MaxEdge.Y);
		f32 zd = getDistanceFromInterval(origin.Z, nb.box.MinEdge.Z, nb.box.MaxEdge.Z);
		return xd + yd + zd;
	}

	inline f32 getDistanceFromInterval(f32 value, f32 min, f32 max) const
	{
		if(value < min)
			return min - value;
		else if(value > max)
			return value - max;
		else
			return 0;
	}

	bool operator()(const Nodebox &a, const Nodebox &b) const
	{
		return getDistance(a) < getDistance(b);
	}
};


collisionMoveResult collisionMoveSimple(Map *map, IGameDef *gamedef,
		f32 pos_max_d, const aabb3f &box_0,
		f32 stepheight, f32 dtime,
		v3f &pos_f, v3f &speed_f, v3f &accel_f)
{
	TimeTaker tt("collisionMoveSimple");

	collisionMoveResult result;

	/*
		Calculate new velocity
	*/
	speed_f += accel_f * dtime;

	/*
		Collect node boxes in movement range
	*/
	std::vector<Nodebox> nodeboxes;
	{
	TimeTaker tt2("collisionMoveSimple collect boxes");

	v3s16 oldpos_i = floatToInt(pos_f, BS);
	v3s16 newpos_i = floatToInt(pos_f + speed_f * dtime, BS);
	s16 min_x = MYMIN(oldpos_i.X, newpos_i.X) + (box_0.MinEdge.X / BS) - 1;
	s16 min_y = MYMIN(oldpos_i.Y, newpos_i.Y) + (box_0.MinEdge.Y / BS) - 1;
	s16 min_z = MYMIN(oldpos_i.Z, newpos_i.Z) + (box_0.MinEdge.Z / BS) - 1;
	s16 max_x = MYMAX(oldpos_i.X, newpos_i.X) + (box_0.MaxEdge.X / BS) + 1;
	s16 max_y = MYMAX(oldpos_i.Y, newpos_i.Y) + (box_0.MaxEdge.Y / BS) + 1;
	s16 max_z = MYMAX(oldpos_i.Z, newpos_i.Z) + (box_0.MaxEdge.Z / BS) + 1;

	for(s16 x = min_x; x <= max_x; x++)
	for(s16 y = min_y; y <= max_y; y++)
	for(s16 z = min_z; z <= max_z; z++)
	{
		try{
			// Object collides into walkable nodes
			MapNode n = map->getNode(v3s16(x,y,z));
			if(gamedef->getNodeDefManager()->get(n).walkable == false)
				continue;

			aabb3f box = getNodeBox(v3s16(x,y,z), BS);
			nodeboxes.push_back(Nodebox(box, false, false));
		}
		catch(InvalidPositionException &e)
		{
			// Collide with unloaded nodes
			aabb3f box = getNodeBox(v3s16(x,y,z), BS);
			nodeboxes.push_back(Nodebox(box, true, false));
		}
	}
	} // tt2

	/*
		Collision detection
	*/

	/*
		Collision uncertainty radius
		Make it a bit larger than the maximum distance of movement
	*/
	f32 d = pos_max_d * 1.1;
	// A fairly large value in here makes moving smoother
	//f32 d = 0.15*BS;

	// This should always apply, otherwise there are glitches
	assert(d > pos_max_d);

	int loopcount = 0;

	while(dtime > BS*1e-10)
	{
		TimeTaker tt3("collisionMoveSimple dtime loop");

		// Avoid infinite loop
		loopcount++;
		if(loopcount >= 100)
		{
			infostream<<"collisionMoveSimple: WARNING: Loop count exceeded, aborting to avoid infiniite loop"<<std::endl;
			dtime = 0;
			break;
		}

		/*
			Sort nodeboxes by distance from (old) object position.
			NOTE: This is a heuristic.
		*/
		NodeboxCompareByDistance comparator(pos_f);
		std::sort(nodeboxes.begin(), nodeboxes.end(), comparator);

		aabb3f movingbox = box_0;
		movingbox.MinEdge += pos_f;
		movingbox.MaxEdge += pos_f;

		int collided = -1;

		/*
			Go through every nodebox
		*/
		for(u32 boxindex = 0; boxindex < nodeboxes.size(); boxindex++)
		{
			// Ignore if already stepped up this nodebox.
			if(nodeboxes[boxindex].is_step_up)
				continue;

			// Find nearest collision of the two boxes (raytracing-like)
			const aabb3f& staticbox = nodeboxes[boxindex].box;
			f32 dtime_tmp;
			collided = axisAlignedCollision(staticbox, movingbox, speed_f, d, dtime_tmp);

			// If it didn't collide, try next nodebox.
			if(collided != -1 && dtime_tmp > dtime)
				collided = -1;
			if(collided == -1)
				continue;

			// Otherwise, a collision occurred.

			// Check for stairs.
			bool step_up = (collided != 1) && // must not be Y direction
					(movingbox.MinEdge.Y + stepheight > staticbox.MaxEdge.Y);

			// Move to the point of collision and reduce dtime by dtime_tmp
			if(dtime_tmp < 0)
			{
				// Handle negative dtime_tmp (can be caused by the d allowance)
				if(!step_up)
				{
					if(collided == 0)
						pos_f.X += speed_f.X * dtime_tmp;
					if(collided == 1)
						pos_f.Y += speed_f.Y * dtime_tmp;
					if(collided == 2)
						pos_f.Z += speed_f.Z * dtime_tmp;
				}
			}
			else
			{
				pos_f += speed_f * dtime_tmp;
				dtime -= dtime_tmp;
			}

			// Set the speed component that caused the collision to zero
			if(step_up)
			{
				// Special case: Handle stairs
				nodeboxes[boxindex].is_step_up = true;
			}
			else if(collided == 0) // X
			{
				speed_f.X = 0;
				result.collides = true;
				result.collides_xz = true;
			}
			else if(collided == 1) // Y
			{
				speed_f.Y = 0;
				result.collides = true;
			}
			else if(collided == 2) // Z
			{
				speed_f.Z = 0;
				result.collides = true;
				result.collides_xz = true;
			}
			break;
		}

		if(collided == -1)
		{
			// No collision with any nodebox.
			pos_f += speed_f * dtime;
			dtime = 0;
		}
	}

	/*
		Final touches: Check if standing on ground, step up stairs.
	*/
	aabb3f box = box_0;
	box.MinEdge += pos_f;
	box.MaxEdge += pos_f;
	for(u32 boxindex = 0; boxindex < nodeboxes.size(); boxindex++)
	{
		const aabb3f& nodebox = nodeboxes[boxindex].box;
		bool is_unloaded = nodeboxes[boxindex].is_unloaded;
		bool is_step_up = nodeboxes[boxindex].is_step_up;

		/*
			See if the object is touching ground.

			Object touches ground if object's minimum Y is near node's
			maximum Y and object's X-Z-area overlaps with the node's
			X-Z-area.

			Use 0.15*BS so that it is easier to get on a node.
		*/
		if(
				nodebox.MaxEdge.X-d > box.MinEdge.X &&
				nodebox.MinEdge.X+d < box.MaxEdge.X &&
				nodebox.MaxEdge.Z-d > box.MinEdge.Z &&
				nodebox.MinEdge.Z+d < box.MaxEdge.Z
		){
			if(is_step_up)
			{
				pos_f.Y += (nodebox.MaxEdge.Y - box.MinEdge.Y);
				box = box_0;
				box.MinEdge += pos_f;
				box.MaxEdge += pos_f;
			}
			if(fabs(nodebox.MaxEdge.Y-box.MinEdge.Y) < 0.15*BS)
			{
				result.touching_ground = true;
				if(is_unloaded)
					result.standing_on_unloaded = true;
			}
		}
	}

	return result;
}

collisionMoveResult collisionMovePrecise(Map *map, IGameDef *gamedef,
		f32 pos_max_d, const aabb3f &box_0,
		f32 stepheight, f32 dtime,
		v3f &pos_f, v3f &speed_f, v3f &accel_f)
{
	TimeTaker tt("collisionMovePrecise");
	infostream<<"start collisionMovePrecise\n";

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


	infostream<<"end collisionMovePrecise\n";
	return final_result;
}
