extends Node

# load the Simple library
onready var data = preload("res://src/native/LTELBinder.gdns").new()
onready var config = preload("res://src/game_config.gd").new()
var is_init = false

# Called when the node enters the scene tree for the first time.
func _ready():
	add_child(data)
	
	# Load it up!
	if !self.config.load_config():
		return
		
	# Setup window title
	OS.set_window_title("LTEL - %s" % self.config.game_name)
	
	is_init = data.init_cshell(self.config.game_data_dir)
# End Func

func _process(delta):
	if is_init == false:
		return
	# End If

	data.game_update(delta)
	
	pass
# End Func

func _unhandled_input(event):
	if is_init == false:
		return
	# End If
	
	if event is InputEventKey:		
		data.on_key_input(event.scancode, event.pressed)
	# End If
# End Func
