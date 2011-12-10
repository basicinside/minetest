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
#include <algorithm>  // std::sort
#include <vector>

// Helper function:
// Checks for collision of a moving aabbox with a static aabbox
// Returns -1 if no collision, 0 if X collision, 1 if Y collision, 2 if Z collision
// The time after which the collision occurs is stored in dtime.
int axisAlignedCollision(
		const aabb3f &staticbox, const aabb3f &movingbox,
		const v3f &speed, f32 &dtime)
{
	// Transform so that staticbox == (0,0,0,1,1,1)
	f32 xoffset = - staticbox.MinEdge.X;
	f32 xscale = 1.0 / (staticbox.MaxEdge.X - staticbox.MinEdge.X);
	f32 yoffset = - staticbox.MinEdge.Y;
	f32 yscale = 1.0 / (staticbox.MaxEdge.Y - staticbox.MinEdge.Y);
	f32 zoffset = - staticbox.MinEdge.Z;
	f32 zscale = 1.0 / (staticbox.MaxEdge.Z - staticbox.MinEdge.Z);

	aabb3f mymovingbox;
	mymovingbox.MinEdge.X = (movingbox.MinEdge.X + xoffset) * xscale;
	mymovingbox.MinEdge.Y = (movingbox.MinEdge.Y + yoffset) * yscale;
	mymovingbox.MinEdge.Z = (movingbox.MinEdge.Z + zoffset) * zscale;
	mymovingbox.MaxEdge.X = (movingbox.MaxEdge.X + xoffset) * xscale;
	mymovingbox.MaxEdge.Y = (movingbox.MaxEdge.Y + yoffset) * yscale;
	mymovingbox.MaxEdge.Z = (movingbox.MaxEdge.Z + zoffset) * zscale;

	v3f myspeed;
	myspeed.X = speed.X * xscale;
	myspeed.Y = speed.Y * yscale;
	myspeed.Z = speed.Z * zscale;

	if(myspeed.X > 0) // Check for collision with X=0 plane
	{
		if(mymovingbox.MaxEdge.X <= 0)
		{
			dtime = - mymovingbox.MaxEdge.X / myspeed.X;
			if(mymovingbox.MinEdge.Y + myspeed.Y * dtime < 1
					&& mymovingbox.MaxEdge.Y + myspeed.Y * dtime > 0
					&& mymovingbox.MinEdge.Z + myspeed.Z * dtime < 1
					&& mymovingbox.MaxEdge.Z + myspeed.Z * dtime > 0)
				return 0;
		}
		else if(mymovingbox.MinEdge.X > 1)
		{
			return -1;
		}
	}
	else if(myspeed.X < 0) // Check for collision with X=1 plane
	{
		if(mymovingbox.MinEdge.X >= 1)
		{
			dtime = (1 - mymovingbox.MinEdge.X) / myspeed.X;
			if(mymovingbox.MinEdge.Y + myspeed.Y * dtime < 1
					&& mymovingbox.MaxEdge.Y + myspeed.Y * dtime > 0
					&& mymovingbox.MinEdge.Z + myspeed.Z * dtime < 1
					&& mymovingbox.MaxEdge.Z + myspeed.Z * dtime > 0)
				return 0;
		}
		else if (mymovingbox.MaxEdge.X < 0)
		{
			return -1;
		}
	}

	if(myspeed.Y > 0) // Check for collision with Y=0 plane
	{
		if(mymovingbox.MaxEdge.Y <= 0)
		{
			dtime = - mymovingbox.MaxEdge.Y / myspeed.Y;
			if(mymovingbox.MinEdge.X + myspeed.X * dtime < 1
					&& mymovingbox.MaxEdge.X + myspeed.X * dtime > 0
					&& mymovingbox.MinEdge.Z + myspeed.Z * dtime < 1
					&& mymovingbox.MaxEdge.Z + myspeed.Z * dtime > 0)
				return 1;
		}
		else if(mymovingbox.MinEdge.Y > 1)
		{
			return -1;
		}
	}
	else if(myspeed.Y < 0) // Check for collision with Y=1 plane
	{
		if(mymovingbox.MinEdge.Y >= 1)
		{
			dtime = (1 - mymovingbox.MinEdge.Y) / myspeed.Y;
			if(mymovingbox.MinEdge.X + myspeed.X * dtime < 1
					&& mymovingbox.MaxEdge.X + myspeed.X * dtime > 0
					&& mymovingbox.MinEdge.Z + myspeed.Z * dtime < 1
					&& mymovingbox.MaxEdge.Z + myspeed.Z * dtime > 0)
				return 1;
		}
		else if (mymovingbox.MaxEdge.Y < 0)
		{
			return -1;
		}
	}

	if(myspeed.Z > 0) // Check for collision with Z=0 plane
	{
		if(mymovingbox.MaxEdge.Z <= 0)
		{
			dtime = - mymovingbox.MaxEdge.Z / myspeed.Z;
			if(mymovingbox.MinEdge.X + myspeed.X * dtime < 1
					&& mymovingbox.MaxEdge.X + myspeed.X * dtime > 0
					&& mymovingbox.MinEdge.Y + myspeed.Y * dtime < 1
					&& mymovingbox.MaxEdge.Y + myspeed.Y * dtime > 0)
				return 2;
		}
		//else if(mymovingbox.MinEdge.Z > 1)
		//{
		//	return -1;
		//}
	}
	else if(myspeed.Z < 0) // Check for collision with Z=1 plane
	{
		if(mymovingbox.MinEdge.Z >= 1)
		{
			dtime = (1 - mymovingbox.MinEdge.Z) / myspeed.Z;
			if(mymovingbox.MinEdge.X + myspeed.X * dtime < 1
					&& mymovingbox.MaxEdge.X + myspeed.X * dtime > 0
					&& mymovingbox.MinEdge.Y + myspeed.Y * dtime < 1
					&& mymovingbox.MaxEdge.Y + myspeed.Y * dtime > 0)
				return 2;
		}
		//else if (mymovingbox.MaxEdge.Z < 0)
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

	Nodebox(const aabb3f &box_, bool is_unloaded_):
		box(box_), is_unloaded(is_unloaded_) {}
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
	collisionMoveResult result;

	/*
		Calculate new velocity
	*/
	v3f newspeed = speed_f + accel_f * dtime;

	/*
		Calculate new position
	*/
	v3f newpos = pos_f + newspeed * dtime;

	/*
		Collect node boxes in movement range
	*/
	std::vector<Nodebox> nodeboxes;

	v3s16 oldpos_i = floatToInt(pos_f, BS);
	v3s16 newpos_i = floatToInt(newpos, BS);
	s16 min_x = MYMIN(oldpos_i.X, newpos_i.X) + (box_0.MinEdge.X / BS) - 2;
	s16 min_y = MYMIN(oldpos_i.Y, newpos_i.Y) + (box_0.MinEdge.Y / BS) - 2;
	s16 min_z = MYMIN(oldpos_i.Z, newpos_i.Z) + (box_0.MinEdge.Z / BS) - 2;
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
			nodeboxes.push_back(Nodebox(box, false));
		}
		catch(InvalidPositionException &e)
		{
			// Collide with unloaded nodes
			aabb3f box = getNodeBox(v3s16(x,y,z), BS);
			nodeboxes.push_back(Nodebox(box, true));
		}
	}

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

	while(dtime > BS*1e-10)
	{
		bool debug = myrand_range(0, 5000) == 42;
		if(debug)
			dstream<<"\n=== collisionMoveSimple debugging ===\n\n";

		v3f oldpos = pos_f;
		v3f newpos = pos_f + newspeed * dtime;

		/*
			Calculate collision box
		*/
		aabb3f oldbox = box_0;
		oldbox.MaxEdge += oldpos;
		oldbox.MinEdge += oldpos;
		aabb3f box = box_0;
		box.MaxEdge += newpos;
		box.MinEdge += newpos;

		result.touching_ground = true;
		result.standing_on_unloaded = true;

		/*
			Sort nodeboxes by distance from (old) object position.
		*/
		NodeboxCompareByDistance comparator(oldpos);
		std::sort(nodeboxes.begin(), nodeboxes.end(), comparator);

		if(debug)
		{
			dstream<<"oldpos: "<<oldpos.X<<","<<oldpos.Y<<","<<oldpos.Z<<"\n";
			dstream<<"newpos: "<<newpos.X<<","<<newpos.Y<<","<<newpos.Z<<"\n\n";

			/*
				Go through every nodebox
			*/
			for(u32 boxindex = 0; boxindex < nodeboxes.size(); boxindex++)
			{
				const aabb3f& nodebox = nodeboxes[boxindex].box;
				bool is_unloaded = nodeboxes[boxindex].is_unloaded;
	
				dstream<<"box "<<boxindex<<": "
					<<nodebox.MinEdge.X<<","<<nodebox.MinEdge.Y<<","<<nodebox.MinEdge.Z
					<<" -- "
					<<nodebox.MaxEdge.X<<","<<nodebox.MaxEdge.Y<<","<<nodebox.MaxEdge.Z
					<<"  "<<(is_unloaded ? "unloaded" : "loaded")<<"\n";
			}
		}


		/*
			Go through every nodebox
		*/
		for(u32 boxindex = 0; boxindex < nodeboxes.size(); boxindex++)
		{
			const aabb3f& nodebox = nodeboxes[boxindex].box;
			//bool is_unloaded = nodeboxes[boxindex].is_unloaded;

			int walking_up_step = 2; // 0 = no, 1 = yes, 2 = inhibited

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
						&& newspeed.dotProduct(dirs[i]) < 0);
				bool positive_axis_collides =
					(nodemin < objectmax && nodemin >= objectmax_old - d
						&& newspeed.dotProduct(dirs[i]) > 0);
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
					newspeed -= newspeed.dotProduct(dirs[i]) * dirs[i];
					newpos -= newpos.dotProduct(dirs[i]) * dirs[i];
					newpos += oldpos.dotProduct(dirs[i]) * dirs[i];
					result.collides = true;
					if(i != 1)
						result.collides_xz = true;
					walking_up_step = 2;  // inhibit
				}

			}

			if(walking_up_step == 1)
			{
				dstream << "walking up step\n";
				newspeed.Y = 0;

				f32 ydiff = nodebox.MaxEdge.Y - box.MinEdge.Y;
				pos_f.Y += ydiff;
				box.MinEdge.Y += ydiff;
				box.MaxEdge.Y += ydiff;
			}
		}

		if(debug)
			dstream<<"\n=== END collisionMoveSimple debugging ===\n\n";

		pos_f = newpos;
		speed_f = newspeed;
		dtime = 0;
	}

	/*
		Final touches: Check if standing on ground
	*/
	aabb3f box = box_0;
	box.MaxEdge += newpos;
	box.MinEdge += newpos;
	for(u32 boxindex = 0; boxindex < nodeboxes.size(); boxindex++)
	{
		const aabb3f& nodebox = nodeboxes[boxindex].box;
		bool is_unloaded = nodeboxes[boxindex].is_unloaded;

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
	}

	return result;
}

collisionMoveResult collisionMovePrecise(Map *map, IGameDef *gamedef,
		f32 pos_max_d, const aabb3f &box_0,
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
