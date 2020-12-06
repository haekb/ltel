extends Spatial

#
# This script gets replaced with a game object
#

enum OT {
	NORMAL = 0,
	MODEL = 1,
	WORLDMODEL = 2,
	SPRITE = 3,
	LIGHT = 4,
	CAMERA = 5,
	PARTICLESYSTEM = 6,
	POLYGRID = 7,
	LINESYSTEM = 8,
	CONTAINER = 9
	_TOTAL = 10,
}

enum Flags {
	ALL_VISIBLE = (1<<0),
	
	MODEL_SHADOW  = (1<<1),
	POLYGRID_UNSIGNED = (1<<1), # Used unsigned bytes
	
	MODEL_TINT = (1<<2),
	
	LIGHT_CASTSHADOWS = (1<<3),
	SPRITE_ROTATEABLE = (1<<3),
	MODEL_GOURAUDSHADE = (1<<3),
	PARTICLE_UPDATEUNSEEN = (1<<3),
	
	LIGHT_SOLIDLIGHT = (1<<4),
	SPRITE_CHROMAKEY = (1<<4), # Doesn't do anything anymore
	MODEL_WIREFRAME = (1<<4),
	PARTICLE_POLYGRID_WASDRAWN = (1<<4),
	
	SPRITE_GLOWSPRITE = (1<<5),
	LIGHT_ONLYLIGHTWORLD = (1<<5),
	MODEL_ENVMAP = (1<<5),
	POLYGRID_ENVMAPONLY = (1<<5),
	
	SPRITE_BIAS = (1<<6),
	LIGHT_DONTLIGHTBACKFACES = (1<<6),
	MODEL_REALLYCLOSE = (1<<6),
	
	LIGHT_FOGLIGHT = (1<<7),
	MODEL_ANIMTRANSITION = (1<<7),
	SPRITE_NOZ = (1<<7),
	
	ALL_FULLPOSRES = (1<<8), # No compression
	
	MODEL_NOLIGHT = (1<<9), # Full bright
	
	ALL_HARDWAREONLY = (1<<10), # Only render on hardware rendering, lol.
	
	ALL_YROTATION = (1<<11), # Compression
	
	ALL_SKYOBJECT = (1<<12),
	
	ALL_SOLID = (1<<13),
	
	WORLDMODEL_CONTAINER_BOXPHYSICS = (1<<14),
	
	ALL_CLIENTNONSOLID = (1<<15),
	
	# Server Only Flags
	ALL_TOUCH_NOTIFY = (1<<16),
	ALL_GRAVITY = (1<<17),
	ALL_STAIRSTEP = (1<<18),
	ALL_MODELKEYS = (1<<19),
	ALL_KEEPALIVE = (1<<20),
	ALL_GOTHRUWORLD = (1<<21),
	ALL_RAYHIT = (1<<22),
	ALL_DONTFOLLOWSTANDING = (1<<23),
	ALL_FORCECLIENTUPDATE = (1<<24),
	ALL_NOSLIDING = (1<<25),
	ALL_POINTCOLLIDE = (1<<26),
	ALL_REMOVEIFOUTSIDE = (1<<27),
	ALL_FORCEOPTIMIZEOBJECT = (1<<28),
	
	# Internal
	ALL_INTERNAL1 = (1<<29), # Did the renderer see the object (not used)
	ALL_INTERNAL2 = (1<<30), # Used by ClientDE::FindObjectsInSphere (not used)
	_LAST = (1<<31),
}

# This is essentially an object create struct
export var base_class_name : String = ""

# Object's name!
export var obj_name : String = ""

# Filename is used for Model, Sound or Sprite. (And also World Model?)
export var file_name : String = ""
# Used for Model
export var skin_name : String = ""

export(OT) var type = OT.NORMAL
#export(Flags, FLAGS) var flags = Flags.ALL_VISIBLE | Flags.ALL_SOLID
# Sorry gotta calculate it yourself :(
export(int) var flags = Flags.ALL_VISIBLE | Flags.ALL_SOLID
export var container_code = 0

export var next_update : float = 0.1
export var deactivation_time : float = 0.0
