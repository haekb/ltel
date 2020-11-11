extends Button


# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var controller : Control
var play_pause : Button

# Called when the node enters the scene tree for the first time.
func _ready():
	self.connect("pressed", self, "_button_pressed")
	self.controller = get_node("/root/Scene/Control")
	self.play_pause = get_node("/root/Scene/Control/PlayPause")
	pass # Replace with function body.

func _button_pressed():
	self.controller.imal.stop_playlist()
	self.controller.playlist_songs = []
	self.play_pause.is_playing = false
	self.play_pause.adjust_text()
# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
