require 'mkmf'

root_dir = File.join(File.dirname(__FILE__), "..", "..", "..", "..", "..")
port_dir = File.join(File.dirname(__FILE__), "..", "port")
lib_sources = ["lora_frame.c", "lora_aes.c", "lora_cmac.c"]

$srcs = ['ext_frame.c'].concat(lib_sources)
$VPATH << File.join(root_dir, "src")
$INCFLAGS << " -I#{File.join(root_dir, "include")} -I#{port_dir}"
$defs << " -DLORA_DEBUG_INCLUDE=\\\"ext_lora_debug.h\\\""

create_makefile('ldl/ext_frame')

