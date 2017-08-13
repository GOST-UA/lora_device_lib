require 'mkmf'

root_dir = File.join(File.dirname(__FILE__), "..", "..", "..", "..", "..")
port_dir = File.join(File.dirname(__FILE__), "..", "port")
lib_sources = Dir.glob(File.join(root_dir, "src", "*.c")).map{|p|File.basename(p)}.delete("lora_radio_sx1272.c")

$srcs = ['ext_lora_device_lib.c'].concat(lib_sources)
$VPATH << File.join(root_dir, "src")
$INCFLAGS << " -I#{File.join(root_dir, "include")} -I#{port_dir}"
$defs << " -DLORA_DEBUG_INCLUDE=\\\"ext_lora_debug.h\\\""

create_makefile('lora_device_lib/ext_lora_device_lib')

