require 'mkmf'

root_dir = File.join(File.dirname(__FILE__), "..", "..", "..", "..", "..")
lib_sources = Dir.glob(File.join(root_dir, "src", "*.c")).map{|p|File.basename(p)}

$srcs = ['ext_lora_device_lib.c'].concat(lib_sources)
$VPATH << File.join(root_dir, "src")
$INCFLAGS << " -I#{File.join(root_dir, "include")}"
$defs << " -DLORA_DEBUG_INCLUDE=\\\"ext_lora_debug.h\\\""

create_makefile('lora_device_lib/ext_lora_device_lib')

