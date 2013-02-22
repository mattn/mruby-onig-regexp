class OnigRegexp
  def self.compile(*args)
    self.new(*args)
  end

  # ISO 15.2.15.7.8
  attr_reader :source

  def self.last_match
    return @last_match
  end

  def ===(str)
    return self.match(str) ? true : false
  end

  def =~(str)
    m = self.match(str)
    return m ? m.begin(0) : nil
  end
end

class OnigMatchData
  # ISO 15.2.16.3.11
  attr_reader :string

  def initialize
    @data = []
    @string = ""
  end

  def push(beg = nil, len = nil)
    if (beg && len)
      @data.push(beg: beg, len: len)
    else
      @data.push(nil)
    end
  end

  # ISO 15.2.16.3.2
  def begin(index)
    d = @data[index]
    return (d && d[:beg])
  end

  # ISO 15.2.16.3.4
  def end(index = 0)
    d = @data[index]
    return (d && (d[:beg] + d[:len]))
  end

  # ISO 15.2.16.3.1
  def [](index)
    d = @data[index]
    return (d && @string.slice(d[:beg], d[:len]))
  end

  # ISO 15.2.16.3.8
  def post_match
    d = @data[0]
    return @string.slice(d[:beg] + d[:len] .. -1)
  end

  # ISO 15.2.16.3.9
  def pre_match
    return @string.slice(0, @data[0][:beg])
  end

  # ISO 15.2.16.3.7
  def offset(index)
    d = @data[index]
    return (d && [ d[:beg], d[:beg] + d[:len] ])
  end

  # ISO 15.2.16.3.13
  def to_s
    return self[0]
  end

  def length
    return @data.length
  end

  def size
    return @data.size
  end
end

class String
  def =~(a)
    begin
      (a.class.to_s == 'String' ?  Regexp.new(a.to_s) : a) =~ self
    rescue
      false
    end
  end
  alias_method :old_sub, :sub
  def sub(*args, &blk)
    if args[0].class.to_s == 'String'
      return blk ? old_sub(*args) {|x| blk.call(x)} : old_sub(*args)
    end
    begin
      m = args[0].match(self)
    rescue
      return self
    end
    return self if m.size == 0
    r = ''
    r += m.pre_match
    r += blk ? blk.call(m[0]) : args[1]
    r += m.post_match
    r
  end
  alias_method :old_gsub, :gsub
  def gsub(*args, &blk)
    if args[0].class.to_s == 'String'
      return blk ? old_gsub(*args) {|x| blk.call(x)} : old_gsub(*args)
    end
    ss = self
    r = ''
    while true
      begin
        m = args[0].match(ss)
      rescue
        break
      end
	  break if !m || m.size == 0
      return r if m.end(0) == 0
      r += m.pre_match
      r += blk ? blk.call(m[0]) : args[1]
      ss = m.post_match
    end
    r += ss
    r
  end
  alias_method :old_split, :split
  def split(*args)
    return old_split(' ') if args[0] == nil
    return old_split(*args) if args[0].class.to_s == 'String'
    ss = self
    r = []
    l = args.size == 2 ? args[1].to_i : 0
    while true
      begin
        m = args[0].match(ss)
      rescue
        break
      end
	  break if !m || m.size == 0
      return r if m.end(0) == 0
      r << m.pre_match
      ss = m.post_match
      l -= 1
      break unless l
    end
    r << ss
    r
  end
  alias_method :old_scan, :scan
  def scan(*args, &blk)
    return old_scan(*args) if args[0].class.to_s == 'String'
    ss = self
    r = []
	while true
      begin
        m = args[0].match(ss)
      rescue
        return []
      end
	  break if !m || m.size == 0
      return r if m.end(0) == 0
      r << m[0]
      ss = m.post_match
    end
    if blk
      r.each do |x|
        blk.call(x)
      end
      return self
    end
    r
  end
end

Regexp = OnigRegexp unless Object.const_defined?(:Regexp)

# This is based on https://github.com/masamitsu-murase/mruby-hs-regexp
