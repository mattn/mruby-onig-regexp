# Rakefile for gem development.

MRUBY_CONFIG=File.expand_path(ENV["MRUBY_CONFIG"] || "dev_build_config.rb")
MRUBY_VERSION=ENV["MRUBY_VERSION"] || "stable"

# Rule to check out the mruby sources indicated in MRUBY_VERSION to
# use for gem development.
file :mruby do
  # If we're using one of the common branches, we can clone it
  # directly and also skip a lot of the history, letting us save some
  # bandwidth.  Otherwise, we need to fetch the whole thing and then
  # checkout the desired commit.

  common_branch = %w{master stable}.include?(MRUBY_VERSION)
  depth = "--depth=1 --branch=#{MRUBY_VERSION}" if common_branch

  sh "git clone #{depth} https://github.com/mruby/mruby.git"
  unless common_branch
    Dir.chdir 'mruby' do
      sh "git fetch --tags"
      rev = %x{git rev-parse #{MRUBY_VERSION}}
      sh "git checkout #{rev}"
    end
  end
end

desc "compile binary"
task :compile => :mruby do
  sh "cd mruby && rake all MRUBY_CONFIG=#{MRUBY_CONFIG}"
end

desc "test"
task :test => :mruby do
  sh "cd mruby && rake all test MRUBY_CONFIG=#{MRUBY_CONFIG}"
end

desc "cleanup"
task :clean do
  sh "cd mruby && rake deep_clean" if File.directory?('mruby')
  sh "rm -rf *.lock test_support/*.mrb tools_ruby/*/*.mrb"
end

task :default => :compile

desc "thorough cleanup; gemdev only"
task :spotless => :clean do
  sh "rm -rf mruby *.lock"
end
