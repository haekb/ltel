extends Node

# TODO!
# http://www.fileformat.info/format/pcx/corion.htm
# https://www.fileformat.info/format/pcx/spec/a10e75307b3a4cc49c3bbe6db4c41fa2/view.htm

class PCXHeader:
	var id
	var version
	var encoding
	var bpp
	var x_start
	var y_start
	var x_end
	var y_end
	var width
	var height
	var palette #[48]
	var reserved_1
	var num_bit_planes
	var bytes_per_line
	var palette_type
	var screen_width
	var screen_height
	var reserved_2 #[54]
	
	func read(f : File):
		self.id = f.get_8()
		self.version = f.get_8()
		self.encoding = f.get_8()
		self.bpp = f.get_8()
		self.x_start = f.get_16()
		self.y_start = f.get_16()
		self.x_end = f.get_16()
		self.y_end = f.get_16()
		self.width = f.get_16()
		self.height = f.get_16()
		self.palette = f.get_buffer(48)
		self.reserved_1 = f.get_8()
		self.num_bit_planes = f.get_8()
		self.bytes_per_line = f.get_16()
		self.palette_type = f.get_16()
		self.screen_width = f.get_16()
		self.screen_height = f.get_16()
		self.reserved_2 = f.get_buffer(54)
		
	# End Func
# End Class

# Called when the node enters the scene tree for the first time.
func _ready():	
	pass 

func load_image(filename : String):
	var pcx_header = PCXHeader.new()
	#print("[Load PCX] Loading %s" % filename)
	
	var file = File.new()
	var result = file.open(filename, File.READ)
	
	if result != OK:
		print("[Load PCX] Error %d opening %s " % [result, filename])
		return
	
	# Read the header..
	pcx_header.read(file)
	
	if pcx_header.id != 10:
		print("Warning potentially unknown PCX id in %s", filename)
		
	if pcx_header.encoding != 1:
		print("Warning a non-RLE pcx (%s) is being loaded", filename)

	var isColour = pcx_header.palette_type == 0 || pcx_header.palette_type == 1 
	
	var dataBuffer := PoolByteArray()
	var width = (pcx_header.x_end - pcx_header.x_start) + 1
	var height = (pcx_header.y_end - pcx_header.y_start) + 1
	#var size = pcx_header.bytes_per_line * pcx_header.num_bit_planes# * height
	
	for y in range(height):
		var size = pcx_header.bytes_per_line * pcx_header.num_bit_planes
		
		# Scanlines have to be even, so it's possible that they pad it out if it's not!
		var scanline_end_pos = pcx_header.bytes_per_line - width

		while (size > 0):
			var byte = file.get_8()
			
			if (byte & 0xC0) != 0xC0:
				if size > scanline_end_pos:
					dataBuffer.append(byte)
				size -=1
				continue
			# End If
			
			var count = byte & 0x3F
			byte = file.get_8()
			
			#size -= count
			while (count > 0):
				if size > scanline_end_pos:
					dataBuffer.append(byte)
				count -= 1
				size -= 1
			# End While
		# End While
	# End For
	
	# TODO: Read palette into new buffer
	var colourBuffer := PoolByteArray()
	
	var monoMap := PoolByteArray()
	var colourMap := PoolVector3Array()
	
	var has_extended_palette = file.get_8()
	
	if has_extended_palette != 0x0C:
		print("PCX (%s) doesn't have extended palette", filename)
		# Load in 16 byte palette
		for i in range(48):
			if isColour:
				colourMap.append( Vector3(file.get_8(), file.get_8(), file.get_8()) )
			else:
				monoMap.append( file.get_8() )
	else:
		# Load in the extended palette
		for i in range(256):
			if isColour:
				colourMap.append( Vector3(file.get_8(), file.get_8(), file.get_8()) )
			else:
				monoMap.append( file.get_8() )
	# End For
	
	var di = 0
	
	# Apply the palette
	while (di < dataBuffer.size() ):
		if isColour:
			colourBuffer.append( colourMap[dataBuffer[di]].x )
			colourBuffer.append( colourMap[dataBuffer[di]].y )
			colourBuffer.append( colourMap[dataBuffer[di]].z )
		else:
			colourBuffer.append( monoMap[dataBuffer[di]] )
		di += 1
	
	file.close()
	
	var image = Image.new()
	
	# Expects width*height*bpp (3)
	image.create_from_data(width, height, false, Image.FORMAT_RGB8, colourBuffer)
	
	var texture = ImageTexture.new()
	
	texture.create_from_image(image)
	
	return texture
# End Func
