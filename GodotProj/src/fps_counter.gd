extends RichTextLabel

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	set_text("FPS: <%.2f>" % Engine.get_frames_per_second())
	pass
