extends Button


# Declare member variables here. Examples:
# var a = 2
# var b = "text"

var controller : Control
var song_list : ItemList
var play_list : ItemList

# Called when the node enters the scene tree for the first time.
func _ready():
	self.connect("pressed", self, "_button_pressed")
	self.controller = get_node("/root/Scene/Control")
	self.song_list = get_node("/root/Scene/Control/SongList")
	self.play_list = get_node("/root/Scene/Control/PlayList")
	
	self.song_list.connect("item_activated", self, "_on_SongList_item_activated")
	
	
	pass # Replace with function body.

func _button_pressed():
	if !self.song_list.is_anything_selected():
		return
	
	var selected_items = self.song_list.get_selected_items()

	for index in selected_items:
		self.play_list.add_item(self.song_list.get_item_text(index))
		pass

	self.controller.playlist_songs = []

	pass


func _on_SongList_item_activated(index):
	self.play_list.add_item(self.song_list.get_item_text(index))
	self.controller.playlist_songs = []
	pass # Replace with function body.
