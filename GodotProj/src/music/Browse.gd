extends Button


# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var data_dir : TextEdit
var file_dialog : FileDialog
# Called when the node enters the scene tree for the first time.
func _ready():
	self.data_dir = get_node("/root/Scene/Control/DataDirectory")
	self.connect("pressed", self, "_button_pressed")
	
	self.file_dialog = FileDialog.new()
	self.file_dialog.access = FileDialog.ACCESS_FILESYSTEM
	self.file_dialog.mode = FileDialog.MODE_OPEN_DIR
	
	self.file_dialog.connect("dir_selected", self, "_dir_selected")
	
	add_child(self.file_dialog)
	pass # Replace with function body.

func _button_pressed():
	self.file_dialog.popup_centered_ratio()

func _dir_selected(dir):
	self.data_dir.text = dir
	pass

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
