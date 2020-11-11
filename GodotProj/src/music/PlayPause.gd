extends Button


# Declare member variables here. Examples:
# var a = 2
# var b = "text"

var controller : Control
var song_list : ItemList
var play_list : ItemList

var is_playing = false

# Called when the node enters the scene tree for the first time.
func _ready():
	self.connect("pressed", self, "_button_pressed")
	self.controller = get_node("/root/Scene/Control")

	self.play_list = get_node("/root/Scene/Control/PlayList")
	
	pass # Replace with function body.

func _button_pressed():

	if self.controller.playlist_songs.size() > 0:
		play_or_pause()
		return
	
	var song_names = []

	for index in range(self.play_list.get_item_count()):
		song_names.append(self.play_list.get_item_text(index))
		
	self.controller.playlist_songs = song_names
	
	self.controller.imal.set_transition("s.sec")
	
	self.controller.imal.create_playlist(self.controller.playlist_songs)
	
	self.controller.imal.play_playlist()
	self.is_playing = true

	self.adjust_text()
	pass

func play_or_pause():

	if self.is_playing:
		self.controller.imal.pause_playlist()
		self.is_playing = false
	else:
		self.controller.imal.resume_playlist()
		self.is_playing = true
	
	self.adjust_text()
	
	
func adjust_text():
	if self.is_playing:
		self.text = "Pause"
	else:
		self.text = "Play"
		
