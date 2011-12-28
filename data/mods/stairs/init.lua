stairs = {
	-- Node will be called stairs:stair_<subname>
	register_stair = function(subname, recipeitem, material, images)
		minetest.register_node("stairs:stair_" .. subname, {
			drawtype = "nodebox",
			tile_images = images,
			inventory_image = images[1] .. "^[brighten",
			paramtype = "light",
			paramtype2 = "facedir_simple",
			is_ground_content = true,
			node_box = {
				type = "fixed",
				fixed = {
					{-0.5, -0.5, -0.5, 0.5, 0, 0.5},
					{-0.5, 0, 0, 0.5, 0.5, 0.5},
				},
			},
			material = material,
		})

		minetest.register_craft({
			output = 'node "stairs:stair_' .. subname .. '" 4',
			recipe = {
				{recipeitem, "", ""},
				{recipeitem, recipeitem, ""},
				{recipeitem, recipeitem, recipeitem},
			},
		})
	end,

	-- Node will be called stairs:slab_<subname>
	register_slab = function(subname, recipeitem, material, images)
		minetest.register_node("stairs:slab_" .. subname, {
			drawtype = "nodebox",
			tile_images = images,
			inventory_image = images[1] .. "^[brighten",
			paramtype = "light",
			paramtype2 = "facedir_simple",
			is_ground_content = true,
			node_box = {
				type = "fixed",
				fixed = {{-0.5, -0.5, -0.5, 0.5, 0, 0.5}},
			},
			selection_box = {
				type = "fixed",
				fixed = {{-0.5, -0.5, -0.5, 0.5, 0, 0.5}},
			},
			material = material,
		})

		minetest.register_craft({
			output = 'node "stairs:slab_' .. subname .. '" 3',
			recipe = {
				{recipeitem, recipeitem, recipeitem},
			},
		})
	end,

	-- Nodes will be called stairs:{stair,slab}_<subname>
	register_stair_and_slab = function(subname, recipeitem, material, images)
		stairs.register_stair(subname, recipeitem, material, images)
		stairs.register_slab(subname, recipeitem, material, images)
	end,
}

stairs.register_stair_and_slab("wood", "node default:wood",
		minetest.digprop_woodlike(0.75),
		{"default_wood.png"})

stairs.register_stair_and_slab("stone", "node default:stone",
		minetest.digprop_stonelike(0.75),
		{"default_stone.png"})

stairs.register_stair_and_slab("cobble", "node default:cobble",
		minetest.digprop_stonelike(0.75),
		{"default_cobble.png"})

stairs.register_stair_and_slab("brick", "node default:brick",
		minetest.digprop_stonelike(0.75),
		{"default_brick.png"})

stairs.register_stair_and_slab("sandstone", "node default:sandstone",
		minetest.digprop_stonelike(0.75),
		{"default_sandstone.png"})
