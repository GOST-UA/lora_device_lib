require 'mkmf'

root_dir = File.join(File.dirname(__FILE__), "..", "..", "..", "..", "..")
port_dir = File.join(File.dirname(__FILE__), "..", "port")
lib_sources = Dir.glob(File.join(root_dir, "src", "*.c")).map{|p|File.basename(p)}.select{|s|not s[/lora_radio_[a-z0-9]+.c/]}

$srcs = ['ext_mac.c'].concat(lib_sources)
$VPATH << File.join(root_dir, "src")
$INCFLAGS << " -I#{File.join(root_dir, "include")} -I#{port_dir}"
$defs << " -DLORA_DEBUG_INCLUDE=\\\"ext_lora_debug.h\\\""

create_makefile('ldl/ext_mac')

