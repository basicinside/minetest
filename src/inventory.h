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

/*
(c) 2010 Perttu Ahola <celeron55@gmail.com>
*/

#ifndef INVENTORY_HEADER
#define INVENTORY_HEADER

#include <iostream>
#include <sstream>
#include <string>
#include "common_irrlicht.h"
#include "debug.h"
#include "mapblockobject.h"
// For g_materials
#include "main.h"

class InventoryItem
{
public:
	InventoryItem();
	virtual ~InventoryItem();
	
	static InventoryItem* deSerialize(std::istream &is);
	
	virtual const char* getName() const = 0;
	// Shall write the name and the parameters
	virtual void serialize(std::ostream &os) = 0;
	// Shall make an exact clone of the item
	virtual InventoryItem* clone() = 0;
#ifndef SERVER
	// Shall return an image to show in the GUI (or NULL)
	virtual video::ITexture * getImage() { return NULL; }
#endif
	// Shall return a text to show in the GUI
	virtual std::string getText() { return ""; }

private:
};

#define MATERIAL_ITEM_MAX_COUNT 99

class MaterialItem : public InventoryItem
{
public:
	MaterialItem(u8 content, u16 count)
	{
		m_content = content;
		m_count = count;
	}
	/*
		Implementation interface
	*/
	virtual const char* getName() const
	{
		return "MaterialItem";
	}
	virtual void serialize(std::ostream &os)
	{
		//os.imbue(std::locale("C"));
		os<<getName();
		os<<" ";
		os<<(unsigned int)m_content;
		os<<" ";
		os<<m_count;
	}
	virtual InventoryItem* clone()
	{
		return new MaterialItem(m_content, m_count);
	}
#ifndef SERVER
	video::ITexture * getImage()
	{
		/*if(m_content == CONTENT_TORCH)
			return g_texturecache.get("torch_on_floor");

		u16 tile = content_tile(m_content, v3s16(1,0,0));
		return g_tile_contents[tile].getTexture(0);*/
		
		if(m_content >= USEFUL_CONTENT_COUNT)
			return NULL;
			
		return g_irrlicht->getTexture(g_content_inventory_textures[m_content]);
	}
#endif
	std::string getText()
	{
		std::ostringstream os;
		os<<m_count;
		return os.str();
	}
	/*
		Special methods
	*/
	u8 getMaterial()
	{
		return m_content;
	}
	u16 getCount()
	{
		return m_count;
	}
	u16 freeSpace()
	{
		if(m_count > MATERIAL_ITEM_MAX_COUNT)
			return 0;
		return MATERIAL_ITEM_MAX_COUNT - m_count;
	}
	void add(u16 count)
	{
		assert(m_count + count <= MATERIAL_ITEM_MAX_COUNT);
		m_count += count;
	}
	void remove(u16 count)
	{
		assert(m_count >= count);
		m_count -= count;
	}
private:
	u8 m_content;
	u16 m_count;
};

class MapBlockObjectItem : public InventoryItem
{
public:
	/*MapBlockObjectItem(MapBlockObject *obj)
	{
		m_inventorystring = obj->getInventoryString();
	}*/
	MapBlockObjectItem(std::string inventorystring)
	{
		m_inventorystring = inventorystring;
	}
	
	/*
		Implementation interface
	*/
	virtual const char* getName() const
	{
		return "MBOItem";
	}
	virtual void serialize(std::ostream &os)
	{
		for(;;)
		{
			size_t t = m_inventorystring.find('|');
			if(t == std::string::npos)
				break;
			m_inventorystring[t] = '?';
		}
		os<<getName();
		os<<" ";
		os<<m_inventorystring;
		os<<"|";
	}
	virtual InventoryItem* clone()
	{
		return new MapBlockObjectItem(m_inventorystring);
	}

#ifndef SERVER
	video::ITexture * getImage();
#endif
	std::string getText();

	/*
		Special methods
	*/
	std::string getInventoryString()
	{
		return m_inventorystring;
	}

	MapBlockObject * createObject(v3f pos, f32 player_yaw, f32 player_pitch);

private:
	std::string m_inventorystring;
};

class InventoryList
{
public:
	InventoryList(std::string name, u32 size);
	~InventoryList();
	void clearItems();
	void serialize(std::ostream &os);
	void deSerialize(std::istream &is);

	InventoryList(const InventoryList &other);
	InventoryList & operator = (const InventoryList &other);

	std::string getName();
	u32 getSize();
	// Count used slots
	u32 getUsedSlots();
	
	// Get pointer to item
	InventoryItem * getItem(u32 i);
	// Returns old item (or NULL). Parameter can be NULL.
	InventoryItem * changeItem(u32 i, InventoryItem *newitem);
	// Delete item
	void deleteItem(u32 i);
	// Adds an item to a suitable place. Returns false if failed.
	bool addItem(InventoryItem *newitem);
	// If possible, adds item to given slot. Returns true on success.
	// Fails when slot is populated by a different kind of item.
	bool addItem(u32 i, InventoryItem *newitem);

	// Decrements amount of every material item
	void decrementMaterials(u16 count);

	void print(std::ostream &o);
	
private:
	core::array<InventoryItem*> m_items;
	u32 m_size;
	std::string m_name;
};

class Inventory
{
public:
	~Inventory();

	void clear();

	Inventory();
	Inventory(const Inventory &other);
	Inventory & operator = (const Inventory &other);
	
	void serialize(std::ostream &os);
	void deSerialize(std::istream &is);

	InventoryList * addList(const std::string &name, u32 size);
	InventoryList * getList(const std::string &name);
	bool deleteList(const std::string &name);
	// A shorthand for adding items
	bool addItem(const std::string &listname, InventoryItem *newitem)
	{
		InventoryList *list = getList(listname);
		if(list == NULL)
			return false;
		return list->addItem(newitem);
	}
	
private:
	// -1 if not found
	s32 getListIndex(const std::string &name);

	core::array<InventoryList*> m_lists;
};

#define IACTION_MOVE 0

struct InventoryAction
{
	static InventoryAction * deSerialize(std::istream &is);
	
	virtual u16 getType() const = 0;
	virtual void serialize(std::ostream &os) = 0;
	virtual void apply(Inventory *inventory) = 0;
};

struct IMoveAction : public InventoryAction
{
	u16 count;
	std::string from_name;
	s16 from_i;
	std::string to_name;
	s16 to_i;
	
	IMoveAction()
	{
		count = 0;
		from_i = -1;
		to_i = -1;
	}
	IMoveAction(std::istream &is)
	{
		std::string ts;

		std::getline(is, ts, ' ');
		count = stoi(ts);

		std::getline(is, from_name, ' ');

		std::getline(is, ts, ' ');
		from_i = stoi(ts);

		std::getline(is, to_name, ' ');

		std::getline(is, ts, ' ');
		to_i = stoi(ts);
	}

	u16 getType() const
	{
		return IACTION_MOVE;
	}

	void serialize(std::ostream &os)
	{
		os<<"Move ";
		os<<count<<" ";
		os<<from_name<<" ";
		os<<from_i<<" ";
		os<<to_name<<" ";
		os<<to_i;
	}

	void apply(Inventory *inventory);
};

#endif
