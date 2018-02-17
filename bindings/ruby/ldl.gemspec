require File.expand_path("../lib/ldl/version", __FILE__)
require 'time'

Gem::Specification.new do |s|
    s.name    = "ldl"
    s.version = LDL::VERSION
    s.date = Date.today.to_s
    s.summary = "A wrapper around lora_device_lib"
    s.author  = "Cameron Harper"
    s.email = "contact@cjh.id.au"
    s.homepage = "https://github.com/cjhdev/lora_device_lib"
    s.files = Dir.glob("ext/**/*.{c,h,rb}") + Dir.glob("lib/**/*.rb") + Dir.glob("test/**/*.rb") + ["rakefile"]
    s.extensions = ["ext/ldl/ext_mac/extconf.rb", "ext/ldl/ext_frame/extconf.rb"]
    s.license = 'MIT'
    s.test_files = Dir.glob("test/**/*.rb")
    s.add_development_dependency 'rake-compiler'
    s.add_development_dependency 'rake'
    s.add_development_dependency 'minitest'
    s.add_development_dependency 'test-unit'
    s.add_runtime_dependency 'pastel'
    s.required_ruby_version = '>= 2.0'
end
