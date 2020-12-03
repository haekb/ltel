extends AnimatedSprite3D

export var animation_to_fix = "default"
var current_frame = -1

func _ready():
	#var _err = self.connect("frame_changed", self, "_frame_changed")
	self.play()
# End Func


func _process(delta):
	if !self.is_playing() or self.current_frame == self.frame:
		return
	
	self._frame_changed()
	self.current_frame = frame

func _frame_changed():
	var material : SpatialMaterial = self.material_override
	var new_texture = self.frames.get_frame(self.animation_to_fix, self.frame)
	
	if new_texture == null:
		return
	
	
	material.albedo_texture = new_texture
# End Func
