extends Control


# Declare member variables here. Examples:
# var a = 2
# var b = "text"

var available_songs = []
var playlist_songs = []

var imal = null

# Called when the node enters the scene tree for the first time.
func _ready():
	var scene = get_node("/root/Scene")
	scene.connect("imal_ready", self, "_imal_ready")
	
	
	
	pass # Replace with function body.

func _imal_ready(imal):
	self.imal = get_node("/root/Scene/IMALBinder")
	
# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
