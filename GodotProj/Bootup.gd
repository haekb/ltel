extends Node

# load the Simple library
onready var data = preload("res://src/native/LTELBinder.gdns").new()

# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	add_child(data)
	print ( data.method('hi') )
	data.init_cshell()
	pass # Replace with function body.

func _process(delta):
	data.game_update(delta)
	pass
# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
