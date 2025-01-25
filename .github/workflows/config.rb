MRuby::Build.new do |conf|
  toolchain :gcc
  enable_debug
  enable_test

  gem "#{MRUBY_ROOT}/.."
end

MRuby::Build.new("onigmo-bundled") do |conf|
  toolchain :gcc
  enable_debug
  enable_test

  gem "#{MRUBY_ROOT}/.." do |g|
    g.bundle_onigmo
  end
end
