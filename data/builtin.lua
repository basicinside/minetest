--
-- This file contains built-in stuff in Minetest implemented in Lua.
--
-- It is always loaded and executed after registration of the C API,
-- before loading and running any mods.
--

function basic_dump2(o)
	if type(o) == "number" then
		return tostring(o)
	elseif type(o) == "string" then
		return string.format("%q", o)
	elseif type(o) == "boolean" then
		return tostring(o)
	elseif type(o) == "function" then
		return "<function>"
	elseif type(o) == "userdata" then
		return "<userdata>"
	elseif type(o) == "nil" then
		return "nil"
	else
		error("cannot dump a " .. type(o))
		return nil
	end
end

function dump2(o, name, dumped)
	name = name or "_"
	dumped = dumped or {}
	io.write(name, " = ")
	if type(o) == "number" or type(o) == "string" or type(o) == "boolean"
			or type(o) == "function" or type(o) == "nil"
			or type(o) == "userdata" then
		io.write(basic_dump2(o), "\n")
	elseif type(o) == "table" then
		if dumped[o] then
			io.write(dumped[o], "\n")
		else
			dumped[o] = name
			io.write("{}\n") -- new table
			for k,v in pairs(o) do
				local fieldname = string.format("%s[%s]", name, basic_dump2(k))
				dump2(v, fieldname, dumped)
			end
		end
	else
		error("cannot dump a " .. type(o))
		return nil
	end
end

function dump(o, dumped)
	dumped = dumped or {}
	if type(o) == "number" then
		return tostring(o)
	elseif type(o) == "string" then
		return string.format("%q", o)
	elseif type(o) == "table" then
		if dumped[o] then
			return "<circular reference>"
		end
		dumped[o] = true
		local t = {}
		for k,v in pairs(o) do
			t[#t+1] = "" .. k .. " = " .. dump(v, dumped)
		end
		return "{" .. table.concat(t, ", ") .. "}"
	elseif type(o) == "boolean" then
		return tostring(o)
	elseif type(o) == "function" then
		return "<function>"
	elseif type(o) == "userdata" then
		return "<userdata>"
	elseif type(o) == "nil" then
		return "nil"
	else
		error("cannot dump a " .. type(o))
		return nil
	end
end

--
-- Item definition helpers
--

minetest.inventorycube = function(img1, img2, img3)
	img2 = img2 or img1
	img3 = img3 or img1
	return "[inventorycube"
			.. "{" .. img1:gsub("%^", "&")
			.. "{" .. img2:gsub("%^", "&")
			.. "{" .. img3:gsub("%^", "&")
end

minetest.get_pointed_thing_position = function(pointed_thing, above)
	if pointed_thing.type == "node" then
		if above then
			-- The position where a node would be placed
			return pointed_thing.above
		else
			-- The position where a node would be dug
			return pointed_thing.under
		end
	elseif pointed_thing.type == "object" then
		obj = pointed.thing.ref
		if obj ~= nil then
			return obj:getpos()
		else
			return nil
		end
	else
		return nil
	end
end

function minetest.item_place(itemstack, placer, pointed_thing)
	pos = minetest.get_pointed_thing_position(pointed_thing, true)
	if pos ~= nil then
		item = itemstack:take_item()
		if item ~= nil then
			minetest.env:add_item(pos, item)
		end
	end
	return itemstack
end

function minetest.item_drop(itemstack, dropper)
	playerpos = dropper:getpos()
	pos = {x=playerpos.x, y=playerpos.y+0.5, z=playerpos.z}
	minetest.env:add_item(pos, itemstack)
	return ""
end

function minetest.item_eat(hp_change)
	return function(itemstack, user, pointed_thing)  -- closure
		if itemstack:take_item() ~= nil then
			user:set_hp(user:get_hp() + hp_change)
		end
		return itemstack
	end
end

--
-- Item definition defaults
--

minetest.nodedef_default = {
	-- Item properties
	type="node",
	-- name intentionally not defined here
	description = "",
	inventory_image = "",
	wield_image = "",
	wield_scale = {x=1,y=1,z=1},
	stack_max = 99,
	dropcount = -1,
	usable = false,
	liquids_pointable = false,
	tool_digging_properties = nil,

	-- Interaction callbacks
	on_place = nil, -- let C handle node placement for now
	on_drop = minetest.item_drop,
	on_use = nil,

	-- Node properties
	drawtype = "normal",
	visual_scale = 1.0,
	tile_images = {""},
	special_materials = {
		{image="", backface_culling=true},
		{image="", backface_culling=true},
	},
	alpha = 255,
	post_effect_color = {a=0, r=0, g=0, b=0},
	paramtype = "none",
	is_ground_content = false,
	sunlight_propagates = false,
	walkable = true,
	pointable = true,
	diggable = true,
	climbable = false,
	buildable_to = false,
	wall_mounted = false,
	--dug_item intentionally not defined here
	extra_dug_item = "",
	extra_dug_item_rarity = 2,
	metadata_name = "",
	liquidtype = "none",
	liquid_alternative_flowing = "",
	liquid_alternative_source = "",
	liquid_viscosity = 0,
	light_source = 0,
	damage_per_second = 0,
	selection_box = {type="regular"},
	material = {
		diggablity = "normal",
		weight = 0,
		crackiness = 0,
		crumbliness = 0,
		cuttability = 0,
		flammability = 0,
	},
}

minetest.craftitemdef_default = {
	type="craft",
	-- name intentionally not defined here
	description = "",
	inventory_image = "",
	wield_image = "",
	wield_scale = {x=1,y=1,z=1},
	stack_max = 99,
	liquids_pointable = false,
	tool_digging_properties = nil,

	-- Interaction callbacks
	on_place = minetest.item_place,
	on_drop = minetest.item_drop,
	--on_use = ...,
}

minetest.tooldef_default = {
	type="tool",
	-- name intentionally not defined here
	description = "",
	inventory_image = "",
	wield_image = "",
	wield_scale = {x=1,y=1,z=1},
	stack_max = 1,
	liquids_pointable = false,
	tool_digging_properties = nil,

	-- Interaction callbacks
	on_place = minetest.item_place,
	on_drop = minetest.item_drop,
	--on_use = ...,
}

minetest.noneitemdef_default = {  -- This is used for the hand and unknown items
	type="none",
	-- name intentionally not defined here
	description = "",
	inventory_image = "",
	wield_image = "",
	wield_scale = {x=1,y=1,z=1},
	stack_max = 99,
	liquids_pointable = false,
	tool_digging_properties = nil,

	-- Interaction callbacks
	--on_place = ...,
	--on_drop = ...,
	--on_use = ...,
}


--
-- Make raw registration functions inaccessible to anyone except builtin.lua
--

local register_item_raw = minetest.register_item_raw
minetest.register_item_raw = nil

local register_alias_raw = minetest.register_alias_raw
minetest.register_item_raw = nil

--
-- Item / entity / ABM registration functions
--

minetest.registered_abms = {}
minetest.registered_entities = {}
minetest.registered_items = {}
minetest.registered_nodes = {}
minetest.registered_craftitems = {}
minetest.registered_tools = {}
minetest.registered_aliases = {}

-- For tables that are indexed by item name:
-- If table[X] does not exist, default to table[minetest.registered_aliases[X]]
local function set_alias_metatable(table)
	setmetatable(table, {
		__index = function(name)
			return rawget(table, minetest.registered_aliases[name])
		end
	})
end
set_alias_metatable(minetest.registered_items)
set_alias_metatable(minetest.registered_nodes)
set_alias_metatable(minetest.registered_craftitems)
set_alias_metatable(minetest.registered_tools)

-- These item names may not be used because they would interfere
-- with legacy itemstrings
local forbidden_item_names = {
	MaterialItem = true,
	MaterialItem2 = true,
	MaterialItem3 = true,
	NodeItem = true,
	node = true,
	CraftItem = true,
	MBOItem = true,
	ToolItem = true,
}

local function check_modname_prefix(name)
	if name:sub(1,1) == ":" then
		-- Escape the modname prefix enforcement mechanism
		return name:sub(2)
	else
		-- Modname prefix enforcement
		local expected_prefix = minetest.get_current_modname() .. ":"
		if name:sub(1, #expected_prefix) ~= expected_prefix then
			error("Name " .. name .. " does not follow naming conventions: " ..
				"\"modname:\" or \":\" prefix required")
		end
		local subname = name:sub(#expected_prefix+1)
		if subname:find("[^abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]") then
			error("Name " .. name .. " does not follow naming conventions: " ..
				"contains unallowed characters")
		end
		return name
	end
end

function minetest.register_abm(spec)
	-- Add to minetest.registered_abms
	minetest.registered_abms[#minetest.registered_abms+1] = spec
end

function minetest.register_entity(name, prototype)
	-- Check name
	if name == nil then
		error("Unable to register entity: Name is nil")
	end
	name = check_modname_prefix(tostring(name))

	prototype.name = name
	prototype.__index = prototype  -- so that it can be used as a metatable

	-- Add to minetest.registered_entities
	minetest.registered_entities[name] = prototype
end

function minetest.register_item(name, itemdef)
	-- Check name
	if name == nil then
		error("Unable to register item: Name is nil")
	end
	name = check_modname_prefix(tostring(name))
	if forbidden_item_names[name] then
		error("Unable to register item: Name is forbidden: " .. name)
	end
	itemdef.name = name

	-- Apply defaults and add to registered_* table
	if itemdef.type == "node" then
		setmetatable(itemdef, {__index = minetest.nodedef_default})
		minetest.registered_nodes[itemdef.name] = itemdef
	elseif itemdef.type == "craft" then
		setmetatable(itemdef, {__index = minetest.craftitemdef_default})
		minetest.registered_craftitems[itemdef.name] = itemdef
	elseif itemdef.type == "tool" then
		setmetatable(itemdef, {__index = minetest.tooldef_default})
		minetest.registered_tools[itemdef.name] = itemdef
	elseif itemdef.type == "none" then
		setmetatable(itemdef, {__index = minetest.noneitemdef_default})
	else
		error("Unable to register item: Type is invalid: " .. dump(itemdef))
	end

	-- Default dug item
	if itemdef.type == "node" and itemdef.dug_item == nil then
		itemdef.dug_item = ItemStack({name=itemdef.name}):to_string()
	end

	-- Legacy stuff
	if itemdef.cookresult_itemstring ~= nil and itemdef.cookresult_itemstring ~= "" then
		--minetest.register_craft({
		--	type="cooking",
		--	output=itemdef.cookresult_itemstring,
		--	recipe=itemdef.name,
		--	cooktime=itemdef.furnace_cooktime
		--})
	end
	if itemdef.furnace_burntime ~= nil and itemdef.furnace_burntime >= 0 then
		--minetest.register_craft({
		--	type="fuel",
		--	recipe=itemdef.name,
		--	burntime=itemdef.furnace_burntime
		--})
	end

	-- Disable all further modifications
	getmetatable(itemdef).__newindex = {}

	minetest.log("Registering item: " .. itemdef.name)
	minetest.registered_items[itemdef.name] = itemdef
	minetest.registered_aliases[itemdef.name] = nil
	register_item_raw(itemdef)
end

function minetest.register_node(name, nodedef)
	nodedef.type = "node"
	minetest.register_item(name, nodedef)
end

function minetest.register_craftitem(name, craftitemdef)
	craftitemdef.type = "craft"

	-- Legacy stuff
	if craftitemdef.inventory_image == nil and craftitemdef.image ~= nil then
		craftitemdef.inventory_image = craftitemdef.image
	end

	minetest.register_item(name, craftitemdef)
end

function minetest.register_tool(name, tooldef)
	tooldef.type = "tool"
	tooldef.stack_max = 1

	-- Legacy stuff
	if tooldef.inventory_image == nil and tooldef.image ~= nil then
		tooldef.inventory_image = tooldef.image
	end
	if tooldef.tool_digging_properties == nil and
	   (tooldef.full_punch_interval ~= nil or
	    tooldef.basetime ~= nil or
	    tooldef.dt_weight ~= nil or
	    tooldef.dt_crackiness ~= nil or
	    tooldef.dt_crumbliness ~= nil or
	    tooldef.dt_cuttability ~= nil or
	    tooldef.basedurability ~= nil or
	    tooldef.dd_weight ~= nil or
	    tooldef.dd_crackiness ~= nil or
	    tooldef.dd_crumbliness ~= nil or
	    tooldef.dd_cuttability ~= nil) then
		tooldef.tool_digging_properties = {
			full_punch_interval = tooldef.full_punch_interval,
			basetime = tooldef.basetime,
			dt_weight = tooldef.dt_weight,
			dt_crackiness = tooldef.dt_crackiness,
			dt_crumbliness = tooldef.dt_crumbliness,
			dt_cuttability = tooldef.dt_cuttability,
			basedurability = tooldef.basedurability,
			dd_weight = tooldef.dd_weight,
			dd_crackiness = tooldef.dd_crackiness,
			dd_crumbliness = tooldef.dd_crumbliness,
			dd_cuttability = tooldef.dd_cuttability,
		}
	end

	minetest.register_item(name, tooldef)
end

function minetest.register_alias(name, convert_to)
	if forbidden_item_names[name] then
		error("Unable to register alias: Name is forbidden: " .. name)
	end
	if minetest.registered_items[name] ~= nil then
		minetest.log("WARNING: Not registering alias, item with same name" ..
			" is already defined: " .. name .. " -> " .. convert_to)
	else
		minetest.log("Registering alias: " .. name .. " -> " .. convert_to)
		minetest.registered_aliases[name] = convert_to
		register_alias_raw(name, convert_to)
	end
end

-- Alias the forbidden item names to "" so they can't be
-- created via itemstrings (e.g. /give)
local name
for name in pairs(forbidden_item_names) do
	minetest.registered_aliases[name] = ""
	register_alias_raw(name, "")
end


-- Deprecated:
-- Aliases for minetest.register_alias (how ironic...)
--minetest.alias_node = minetest.register_alias
--minetest.alias_tool = minetest.register_alias
--minetest.alias_craftitem = minetest.register_alias

--
-- Built-in node definitions. Also defined in C.
--

minetest.register_item(":", {
	type = "none",
	wield_image = "wieldhand.png",
	tool_digging_properties = {
		full_punch_interval = 2.0,
		basetime = 0.5,
		dt_weight = 1,
		dt_crackiness = 0,
		dt_crumbliness = -1,
		dt_cuttability = 0,
		basedurability = 50,
		dd_weight = 0,
		dd_crackiness = 0,
		dd_crumbliness = 0,
		dd_cuttability = 0,
	}
})

minetest.register_item(":unknown", {
	type = "none",
	description = "Unknown Item",
	on_place = minetest.item_place,
	on_drop = minetest.item_drop,
})

minetest.register_node(":air", {
	description = "Air (you hacker you!)",
	drawtype = "airlike",
	paramtype = "light",
	sunlight_propagates = true,
	walkable = false,
	pointable = false,
	diggable = false,
	buildable_to = true,
	air_equivalent = true,
})

minetest.register_node(":ignore", {
	description = "Ignore (you hacker you!)",
	drawtype = "airlike",
	paramtype = "none",
	sunlight_propagates = false,
	walkable = false,
	pointable = false,
	diggable = false,
	buildable_to = true, -- A way to remove accidentally placed ignores
	air_equivalent = true,
})

--
-- Default material types
--

function minetest.digprop_constanttime(time)
	return {
		diggability = "constant",
		constant_time = time,
	}
end

function minetest.digprop_stonelike(toughness)
	return {
		diggablity = "normal",
		weight = toughness * 5,
		crackiness = 1,
		crumbliness = -0.1,
		cuttability = -0.2,
	}
end

function minetest.digprop_dirtlike(toughness)
	return {
		diggablity = "normal",
		weight = toughness * 1.2,
		crackiness = 0,
		crumbliness = 1.2,
		cuttability = -0.4,
	}
end

function minetest.digprop_gravellike(toughness)
	return {
		diggablity = "normal",
		weight = toughness * 2,
		crackiness = 0.2,
		crumbliness = 1.5,
		cuttability = -1.0,
	}
end

function minetest.digprop_woodlike(toughness)
	return {
		diggablity = "normal",
		weight = toughness * 1.0,
		crackiness = 0.75,
		crumbliness = -1.0,
		cuttability = 1.5,
	}
end

function minetest.digprop_leaveslike(toughness)
	return {
		diggablity = "normal",
		weight = toughness * (-0.5),
		crackiness = 0,
		crumbliness = 0,
		cuttability = 2.0,
	}
end

function minetest.digprop_glasslike(toughness)
	return {
		diggablity = "normal",
		weight = toughness * 0.1,
		crackiness = 2.0,
		crumbliness = -1.0,
		cuttability = -1.0,
	}
end

--
-- Creative inventory
--

minetest.creative_inventory = {}

minetest.add_to_creative_inventory = function(itemstring)
	table.insert(minetest.creative_inventory, itemstring)
end

--
-- Callback registration
--

local function make_registration()
	local t = {}
	local registerfunc = function(func) table.insert(t, func) end
	return t, registerfunc
end

minetest.registered_on_chat_messages, minetest.register_on_chat_message = make_registration()
minetest.registered_globalsteps, minetest.register_globalstep = make_registration()
minetest.registered_on_placenodes, minetest.register_on_placenode = make_registration()
minetest.registered_on_dignodes, minetest.register_on_dignode = make_registration()
minetest.registered_on_punchnodes, minetest.register_on_punchnode = make_registration()
minetest.registered_on_generateds, minetest.register_on_generated = make_registration()
minetest.registered_on_newplayers, minetest.register_on_newplayer = make_registration()
minetest.registered_on_respawnplayers, minetest.register_on_respawnplayer = make_registration()

-- END
