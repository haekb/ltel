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
		
	# Grab the profile
	var profile = config.get_value("Game", "profile", "Game_Shogo")
	
	print("[GameConfig] Loading game profile %s" % profile)
	
	self.game_name = config.get_value(profile, "name", self.game_name)
	self.game_data_dir = config.get_value(profile, "data_dir", self.game_data_dir)
	
	print("[GameConfig] Found config for %s, data directory: %s" % [self.game_name, self.game_data_dir])
	
	return true
# End Func
