extends Button


# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var controller : Control
var data_dir : TextEdit
var play_list : ItemList
var play_pause : Button

# Called when the node enters the scene tree for the first time.
func _ready():
	self.connect("pressed", self, "_button_pressed")
	self.controller = get_node("/root/Scene/Control")
	self.data_dir = get_node("/root/Scene/Control/DataDirectory") as TextEdit
	self.play_list = get_node("/root/Scene/Control/PlayList")
	self.play_pause = get_node("/root/Scene/Control/PlayPause")
	pass # Replace with function body.

func _button_pressed():
	print ("Setting data directory to ",data_dir.text)
	if !self.controller.imal.set_data_directory(data_dir.text):
		print ("Failed to set data dir!")
		return
	
	# Open the directory and scan for some dls/sty files!
	var dir = Directory.new()
	if dir.open(data_dir.text) != OK:
		print ("Failed to open data dir!")
		return
		
	var dls = ""
	var sty = ""
	var file_list = []
	
	dir.list_dir_begin(true)
	var file_name = dir.get_next()
	while file_name != "":
		if dir.current_is_dir():
			continue
			
		#if dls != "" and sty != "":
		#	break
		
		if ".dls" in file_name:
			dls = file_name
		
		if "init.sty" in file_name:
			sty = file_name
			
		if ".sec" in file_name:
			file_list.append(file_name)

		file_name = dir.get_next()
	# End

	
	if !self.controller.imal.setup_instruments(dls, sty):
		print ("Failed to setup instruments!")
		return
		
	var song_list := get_node("/root/Scene/Control/SongList") as ItemList 
	play_list.clear()
	song_list.clear()
	self.play_pause.is_playing = false
	self.play_pause.adjust_text()
	
	self.controller.available_songs.clear()
	
	for song in file_list:
		song_list.add_item(song)
		
	self.controller.available_songs = song_list
	

	print ("Finished loading!")

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
