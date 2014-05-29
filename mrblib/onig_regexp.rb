class OnigRegexp
  @memo = {}

  # ISO 15.2.15.6.1
  def self.compile(*args)
    as = args.to_s
    unless @memo.key? as
      @memo[as] = self.new(*args)
    end
    @memo[as]
  end

  # ISO 15.2.15.6.3
  def self.last_match
    @last_match
  end

  # ISO 15.2.15.7.2
  def initialize_copy(other)
    initialize(other.source, other.options)
  end

  # ISO 15.2.15.7.4
  def ===(str)
    not self.match(str).nil?
  end

  # ISO 15.2.15.7.5
  def =~(str)
    m = self.match(str)
    m ? m.begin(0) : nil
  end

  # ISO 15.2.15.7.8
  attr_reader :source
end

class String
  # ISO 15.2.10.5.5
  def =~(a)
    begin
      (a.class.to_s == 'String' ?  Regexp.new(a.to_s) : a) =~ self
    rescue
      false
    end
  end

  # redefine methods with oniguruma regexp version
  [:sub, :gsub, :split, :scan].each do |v|
    alias_method "string_#{v}".to_sym, v
    alias_method v, "onig_regexp_#{v}".to_sym
  end
end

Regexp = OnigRegexp unless Object.const_defined?(:Regexp)
MatchData = OnigMatchData unless Object.const_defined? :MatchData

# This is based on https://github.com/masamitsu-murase/mruby-hs-regexp
