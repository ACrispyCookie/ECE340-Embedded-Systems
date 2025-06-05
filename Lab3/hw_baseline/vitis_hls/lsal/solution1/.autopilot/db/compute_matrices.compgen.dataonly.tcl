# This script segment is generated automatically by AutoPilot

set axilite_register_dict [dict create]
set port_control {
string1 { 
	dir I
	width 64
	depth 1
	mode ap_none
	offset 16
	offset_end 27
}
string2 { 
	dir I
	width 64
	depth 1
	mode ap_none
	offset 28
	offset_end 39
}
max_index { 
	dir I
	width 64
	depth 1
	mode ap_none
	offset 40
	offset_end 51
}
similarity_matrix { 
	dir I
	width 64
	depth 1
	mode ap_none
	offset 52
	offset_end 63
}
direction_matrix { 
	dir I
	width 64
	depth 1
	mode ap_none
	offset 64
	offset_end 75
}
N { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 76
	offset_end 83
}
M { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 84
	offset_end 91
}
ap_start { }
ap_done { }
ap_ready { }
ap_continue { }
ap_idle { }
}
dict set axilite_register_dict control $port_control


