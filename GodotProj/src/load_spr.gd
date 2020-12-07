extends Node

const texture_builder := preload("res://Addons/DTXReader/TextureBuilder.gd")
const animated_sprite_3d_fix := preload("res://src/animated_sprite_3d_material_override_fix.gd")

class SPR:
	var texture_count
	var framerate
	var unk_1
	var unk_2
	var unk_3
	var textures = []
	
	func read(f : File):
		self.texture_count = f.get_32()
		self.framerate = f.get_32()
		self.unk_1 = f.get_32()
		self.unk_2 = f.get_32()
		self.unk_3 = f.get_32()
		
		for i in range(self.texture_count):
			textures.append( self.read_string(f, true) )
		# End For
		
	# End Func
	
	func read_string(file : File, is_length_a_short = true):
		var length = 0
		if is_length_a_short:
			length = file.get_16() 
		else:
			length = file.get_32() # Sometimes it's 32-bit...
		# End If
			
		return file.get_buffer(length).get_string_from_ascii()
	# End Func
# End Class

func build(path : String, filename : String):
	var builder = texture_builder.new()
	
	var sprite = SPR.new()
	print("[Load SPR] Loading %s" % filename)
	
	var file = File.new()
	var result = file.open(filename, File.READ)
	
	if result != OK:
		print("[Load SPR] Error %d opening %s " % [result, filename])
		return
	
	# Read the header..
	sprite.read(file)
	
	# Setup our fancy material
	var material : SpatialMaterial = SpatialMaterial.new()
	material.flags_unshaded = true
	material.params_billboard_mode = SpatialMaterial.BILLBOARD_ENABLED
	material.params_blend_mode = SpatialMaterial.BLEND_MODE_ADD
	material.params_specular_mode = SpatialMaterial.SPECULAR_DISABLED
	
	# If we only need one frame, then just create a regular sprite 3d
	if sprite.texture_count == 1:
		var tex = builder.build(path + sprite.textures[0], [])
		material.albedo_texture = tex
		
		var sprite3d = Sprite3D.new()
		sprite3d.material_override = material
		sprite3d.double_sided = false
		
		sprite3d.name = filename
		sprite3d.texture = tex
		
		return sprite3d
	# End If
	
	var sprite3d = AnimatedSprite3D.new()
	sprite3d.material_override = material
	sprite3d.double_sided = false
	
	sprite3d.set_script(animated_sprite_3d_fix)
	
	var frames = SpriteFrames.new()
	
	var animation_name = "default"
	frames.set_animation_speed(animation_name, sprite.framerate)
	frames.set_animation_loop(animation_name, true)
	
	for i in range(sprite.texture_count):
		var texture = builder.build(path + sprite.textures[i], [])
		frames.add_frame(animation_name, texture)
	# End For
	
	sprite3d.frames = frames
	
	sprite3d.name = filename
	
	return sprite3d
# End Func
