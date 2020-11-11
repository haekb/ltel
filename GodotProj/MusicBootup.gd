extends Node

# load the Simple library
onready var data = preload("res://src/native/IMALBinder.gdns").new()
onready var config = preload("res://src/game_config.gd").new()
var is_init = false

# Called when the node enters the scene tree for the first time.
func _ready():
	data.set_name("IMALBinder")
	add_child(data)
	
	# Load it up!
	if !self.config.load_config():
		return
		
	# Setup window title
	OS.set_window_title("IMAL - %s" % self.config.game_name)
	
	is_init = data.setup_ima(self.config.game_exe_dir)
# End Func

