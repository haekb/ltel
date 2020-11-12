extends Node

# load the Simple library
onready var data = preload("res://src/native/IMALBinder.gdns").new()
onready var config = preload("res://src/game_config.gd").new()
var is_init = false

signal imal_ready

# Called when the node enters the scene tree for the first time.
func _ready():
	data.set_name("IMALBinder")
	add_child(data)
		
	# Setup window title
	OS.set_window_title("Lithtech 1.0 - Music Player")
	
	is_init = data.setup_ima("")
	
	emit_signal("imal_ready", data)
# End Func

