require File.expand_path("../lib/lora_device_lib/version", __FILE__)

Gem::Specification.new do |s|
    s.name    = "lora_device_lib"
    s.version = LDL::VERSION
    s.date = Date.today.to_s
    s.summary = "A wrapper"
    s.author  = "Cameron Harper"
    s.email = "contact@cjh.id.au"
    s.homepage = "https://github.com/cjhdev/lora_device_lib"
    s.files = Dir.glob("ext/**/*.{c,h,rb}") + Dir.glob("lib/**/*.rb") + Dir.glob("test/**/*.rb") + ["rakefile"]
    s.extensions = ["ext/lora_device_lib/ext_lora_device_lib/extconf.rb"]
    s.license = 'MIT'
    s.test_files = Dir.glob("test/**/*.rb")
    s.add_development_dependency 'rake-compiler'
    s.add_development_dependency 'rake'
    s.add_development_dependency 'test-unit'
    s.required_ruby_version = '>= 2.0'
end
