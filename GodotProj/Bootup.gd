extends Node

# load the Simple library
onready var data = preload("res://src/native/LTELBinder.gdns").new()
var is_init = false

# Called when the node enters the scene tree for the first time.
func _ready():
	add_child(data)
	is_init = data.init_cshell()
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
