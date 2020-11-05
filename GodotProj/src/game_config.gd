extends Node

var game_name = "LTEL Project"
var game_data_dir = "./"

# Called when the node enters the scene tree for the first time.
func load_config():
	var config = ConfigFile.new()
	var err = config.load("./game.cfg")
	if err != OK:
		print("Couldn't find game.cfg!")
		return false
	
	self.game_name = config.get_value("Game", "name", self.game_name)
	self.game_data_dir = config.get_value("Game", "data_dir", self.game_data_dir)
	return true
# End Func
