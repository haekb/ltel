extends Button



var controller : Control
var song_list : ItemList
var play_list : ItemList

func sort_desc(a, b):
		if a > b:
			return true
		return false

# Called when the node enters the scene tree for the first time.
func _ready():
	self.connect("pressed", self, "_button_pressed")
	self.controller = get_node("/root/Scene/Control")
	self.song_list = get_node("/root/Scene/Control/SongList")
	self.play_list = get_node("/root/Scene/Control/PlayList")
	
	self.play_list.connect("item_activated", self, "_on_PlayList_item_activated")
	
	
	pass # Replace with function body.

func _button_pressed():
	if !self.play_list.is_anything_selected():
		return
	
	var selected_items = self.play_list.get_selected_items()
	
	# We need to sort in descending order to remove properly!
	var array_items = []
	for index in selected_items:
		array_items.append(index)
	
	array_items.sort_custom(self, "sort_desc")

	for index in array_items:
		self.play_list.remove_item(index)
		pass
		
	self.controller.playlist_songs = []

	pass


func _on_PlayList_item_activated(index):
	self.play_list.remove_item(index)
	self.controller.playlist_songs = []
	pass # Replace with function body.
