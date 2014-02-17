MRuby::Gem::Specification.new('mruby-onig-regexp') do |spec|
  spec.license = 'MIT'
  spec.authors = 'mattn'

  spec.linker.libraries << ['onig']
  next if ENV['OS'] == 'Windows_NT'

  require 'rubygems'
  require 'net/http'
  require 'libarchive'
  require 'open3'

  version = '5.9.5'
  oniguruma_dir = "#{build_dir}/onig-#{version}"
  oniguruma_lib = libfile "#{oniguruma_dir}/.libs/libonig"
  header = "#{oniguruma_dir}/oniguruma.h"

  task :clean do
    FileUtils.rm_rf [oniguruma_dir]
  end

  file header do |t|
    FileUtils.mkdir_p oniguruma_dir

    _pp 'getting', "onig-#{version}"

    Dir.chdir("#{oniguruma_dir}/..") do
      Net::HTTP.start('www.geocities.jp') do |http|
        Archive.read_open_memory(http.get("/kosako3/oniguruma/archive/onig-#{version}.tar.gz").body,
                                 Archive::COMPRESSION_GZIP, Archive::FORMAT_TAR) do |arch|
          while ent = arch.next_header
            next if ent.directory?

            FileUtils.mkdir_p File.dirname ent.pathname
            File.open(ent.pathname, 'w+') do |f|
              arch.read_data { |d| f.write d }
            end
            File.chmod ent.mode, ent.pathname
          end
        end
      end
    end
  end

  def run_command(env, command)
    STDOUT.sync = true
    Open3.popen2e(env, command) do |stdin, stdout, thread|
      print stdout.read
      fail "#{command} failed" if thread.value != 0
    end
  end

  file oniguruma_lib => header do |t|
    Dir.chdir(oniguruma_dir) do
      e = {
        'CC' => "#{spec.build.cc.command} #{spec.build.cc.flags.join(' ')}",
        'CXX' => "#{spec.build.cxx.command} #{spec.build.cxx.flags.join(' ')}",
        'LD' => "#{spec.build.linker.command} #{spec.build.linker.flags.join(' ')}",
        'AR' => spec.build.archiver.command }
      _pp 'autotools', oniguruma_dir
      run_command e, './autogen.sh' if File.exists? 'autogen.sh'
      run_command e, './configure --disable-shared --enable-static'
      run_command e, 'make'
    end
  end

  file "#{dir}/src/mruby_onig_regexp.c" => oniguruma_lib
  spec.cc.include_paths << oniguruma_dir
  spec.linker.library_paths << File.dirname(oniguruma_lib)
  spec.linker.libraries << 'onig'
end
