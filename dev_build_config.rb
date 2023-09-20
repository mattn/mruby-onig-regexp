MRuby::Build.new do |conf|

  conf.disable_presym

  conf.toolchain()

  conf.gembox 'default'
  conf.gem core: 'mruby-bin-debugger'
  conf.gem File.expand_path(File.dirname(__FILE__))

  conf.enable_debug

  conf.enable_test
  conf.enable_bintest

  if ENV['DEBUG'] == 'true'
    conf.cc.defines = %w(MRB_ENABLE_DEBUG_HOOK)
  end
end
