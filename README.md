# mruby-onig-regexp

[![Build Status](https://travis-ci.org/mattn/mruby-onig-regexp.svg)](https://travis-ci.org/mattn/mruby-onig-regexp)

## install by mrbgems
```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem :github => 'mattn/mruby-onig-regexp'
end
```

If `libonig` is already installed on your system, the gem will use it;
otherwise, it will build a local copy from source code and statically
link to it.

You can force the gem to always use its own version of the library by
setting `MRUBY_ONIGMO_USE_INTERNAL=1` in the environment.


## Example
```ruby

def matchstr(str)
  reg = Regexp.compile("abc")

  if reg =~ str then
    p "match"
  else
    p "not match"
  end
end

matchstr("abcdef") # => match
matchstr("ghijkl") # => not match
matchstr("xyzabc") # => match
```

## License

MIT

### License of Onigmo
BSD licensed.

    Onigmo (Oniguruma-mod)  --  (C) K.Takata <kentkt AT csc DOT jp>
    Oniguruma  ----   (C) K.Kosako <sndgk393 AT ybb DOT ne DOT jp>

## Author

Yasuhiro Matsumoto (a.k.a mattn)
