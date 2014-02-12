
# Constant
assert("OnigRegexp::CONSTANT") do
  OnigRegexp::IGNORECASE == 1 and OnigRegexp::EXTENDED == 2 and OnigRegexp::MULTILINE == 4
end


# Class method
assert("OnigRegexp.new") do
  OnigRegexp.new(".*") and OnigRegexp.new(".*", OnigRegexp::MULTILINE)
end

# Instance method
assert("OnigRegexp#==") do
  reg1 = reg2 = OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  reg3 = OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  reg4 = OnigRegexp.new("(https://[^/]+)[-a-zA-Z0-9./]+")

  reg1 == reg2 and reg1 == reg3 and !(reg1 == reg4)
end

assert("OnigRegexp#===") do
  reg = OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  assert_true reg === "http://example.com"
  assert_false reg === "htt://example.com"
end

# TODO =~

assert("OnigRegexp#casefold?") do
  assert_false OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", OnigRegexp::MULTILINE).casefold?
  assert_true OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", OnigRegexp::IGNORECASE | OnigRegexp::EXTENDED).casefold?
  assert_true OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", OnigRegexp::MULTILINE | OnigRegexp::IGNORECASE).casefold?
  assert_false OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+").casefold?
  assert_true OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", true).casefold?
end

assert("OnigRegexp#match") do
  reg = OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  assert_false reg.match("http://masamitsu-murase.12345/hoge.html").nil?
  assert_nil reg.match("http:///masamitsu-murase.12345/hoge.html")
end

assert("OnigRegexp#source") do
  str = "(https?://[^/]+)[-a-zA-Z0-9./]+"
  reg = OnigRegexp.new(str)

  reg.source == str
end

# Extended patterns.
assert("OnigRegexp#match (no flags)") do
  [
    [ ".*", "abcd\nefg", "abcd" ],
    [ "^a.", "abcd\naefg", "ab" ],
    [ "^a.", "bacd\naefg", "ae" ],
    [ ".$", "bacd\naefg", "d" ]
  ].each do |reg, str, result|
    m = OnigRegexp.new(reg).match(str)
    assert_equal result, m[0] if assert_false m.nil?
  end
end

assert("OnigRegexp#match (multiline)") do
  patterns = [
    [ OnigRegexp.new(".*", OnigRegexp::MULTILINE), "abcd\nefg", "abcd\nefg" ]
  ]

  patterns.all?{ |reg, str, result| reg.match(str)[0] == result }
end

assert("OnigRegexp#match (ignorecase)") do
  [
    [ "aBcD", "00AbcDef", "AbcD" ],
    [ "0x[a-f]+", "00XaBCdefG", "0XaBCdef" ],
    [ "0x[^c-f]+", "00XaBCdefG", "0XaB" ]
  ].each do |reg, str, result|
    m = OnigRegexp.new(reg, OnigRegexp::IGNORECASE|OnigRegexp::EXTENDED).match(str)
    assert_equal result, m[0] if assert_false m.nil?
  end
end

assert('OnigRegexp.version') do
  OnigRegexp.version.kind_of? String
end

def onig_match_data_example
  OnigRegexp.new('(\w+)(\w)').match('+aaabb-')
end

assert('OnigMatchData.new') do
  assert_raise(NoMethodError) { OnigMatchData.new('aaa', 'i') }
end

assert('OnigMatchData#[]') do
  m = onig_match_data_example
  assert_equal 'aaabb', m[0]
  assert_equal 'aaab', m[1]
  assert_equal 'b', m[2]
  assert_nil m[3]

  m = OnigRegexp.new('(?<name>\w\w)').match('aba')
  assert_raise(ArgumentError) { m[[]] }
  assert_raise(IndexError) { m['nam'] }
  assert_equal 'ab', m[:name]
  assert_equal 'ab', m['name']
  assert_equal 'ab', m[1]
end

assert('OnigMatchData#begin') do
  m = onig_match_data_example
  assert_equal 1, m.begin(0)
  assert_equal 1, m.begin(1)
  assert_raise(IndexError) { m.begin 3 }
end

assert('OnigMatchData#captures') do
  m = onig_match_data_example
  assert_equal ['aaab', 'b'], m.captures
end

assert('OnigMatchData#end') do
  m = onig_match_data_example
  assert_equal 6, m.end(0)
  assert_equal 5, m.end(1)
  assert_raise(IndexError) { m.end 3 }
end

assert('OnigMatchData#initialize_copy') do
  m = onig_match_data_example
  c = m.dup
  assert_equal m.to_a, c.to_a
end

assert('OnigMatchData#length') do
  assert_equal 3, onig_match_data_example.length
end

assert('OnigMatchData#offset') do
  assert_equal [1, 6], onig_match_data_example.offset(0)
  assert_equal [1, 5], onig_match_data_example.offset(1)
end

assert('OnigMatchData#post_match') do
  assert_equal '-', onig_match_data_example.post_match
end

assert('OnigMatchData#pre_match') do
  assert_equal '+', onig_match_data_example.pre_match
end

assert('OnigMatchData#size') do
  assert_equal 3, onig_match_data_example.length
end

assert('OnigMatchData#string') do
  assert_equal '+aaabb-', onig_match_data_example.string
end

assert('OnigMatchData#to_a') do
  assert_equal ['aaabb', 'aaab', 'b'], onig_match_data_example.to_a
end

assert('OnigMatchData#to_s') do
  assert_equal 'aaabb', onig_match_data_example.to_s
end

assert('OnigMatchData#regexp') do
  assert_equal '(\w+)(\w)', onig_match_data_example.regexp.source
end

assert('Invalid regexp') do
  assert_raise(ArgumentError) { OnigRegexp.new '[aio' }
end

assert('String#onig_regexp_gsub') do
  test_str = 'hello mruby'
  assert_equal 'h*ll* mr*by', test_str.onig_regexp_gsub(OnigRegexp.new('[aeiou]'), '*')
  assert_equal 'h<e>ll<o> mr<u>by', test_str.onig_regexp_gsub(OnigRegexp.new('([aeiou])'), '<\1>')
  assert_equal 'h e l l o  m r u b y ', test_str.onig_regexp_gsub(OnigRegexp.new('\w')) { |v| v + ' ' }
  assert_equal 'h{e}ll{o} mr{u}by', test_str.onig_regexp_gsub(OnigRegexp.new('(?<hoge>[aeiou])'), '{\k<hoge>}')
  assert_raise(IndexError) { test_str.onig_regexp_gsub(OnigRegexp.new('(mruby)'), '<\2>') }
end

assert('String#onig_regexp_scan') do
  test_str = 'mruby world'
  assert_equal ['mruby', 'world'], test_str.onig_regexp_scan(OnigRegexp.new('\w+'))
  assert_equal ['mru', 'by ', 'wor'], test_str.onig_regexp_scan(OnigRegexp.new('...'))
  assert_equal [['mru'], ['by '], ['wor']], test_str.onig_regexp_scan(OnigRegexp.new('(...)'))
  assert_equal [['mr', 'ub'], ['y ', 'wo']], test_str.onig_regexp_scan(OnigRegexp.new('(..)(..)'))

  result = []
  assert_equal test_str, test_str.onig_regexp_scan(OnigRegexp.new('\w+')) { |v| result << "<<#{v}>>" }
  assert_equal ['<<mruby>>', '<<world>>'], result

  result = ''
  assert_equal test_str, test_str.onig_regexp_scan(OnigRegexp.new('(.)(.)')) { |x, y| result += y; result += x }
  assert_equal 'rmbu yowlr', result
end

assert('String#onig_regexp_sub') do
  test_str = 'hello mruby'
  assert_equal 'h*llo mruby', test_str.onig_regexp_sub(OnigRegexp.new('[aeiou]'), '*')
  assert_equal 'h<e>llo mruby', test_str.onig_regexp_sub(OnigRegexp.new('([aeiou])'), '<\1>')
  assert_equal 'h ello mruby', test_str.onig_regexp_sub(OnigRegexp.new('\w')) { |v| v + ' ' }
  assert_equal 'h{e}llo mruby', test_str.onig_regexp_sub(OnigRegexp.new('(?<hoge>[aeiou])'), '{\k<hoge>}')
end

assert('String#onig_regexp_split') do
  test_str = 'cute mruby cute'
  assert_equal ['cute', 'mruby', 'cute'], test_str.onig_regexp_split
  assert_equal ['cute', 'mruby', 'cute'], test_str.onig_regexp_split(OnigRegexp.new(' '))

  prev_splitter = $;
  $; = OnigRegexp.new ' \w'
  assert_equal ['cute', 'ruby', 'ute'], test_str.onig_regexp_split
  $; = 't'
  assert_equal ['cu', 'e mruby cu', 'e'], test_str.onig_regexp_split
  $; = prev_splitter

  assert_equal ['h', 'e', 'l', 'l', 'o'], 'hello'.onig_regexp_split(OnigRegexp.new(''))
  assert_equal ['h', 'e', 'llo'], 'hello'.onig_regexp_split(OnigRegexp.new(''), 3)
  # TODO: assert_equal ['h', 'i', 'd', 'a', 'd'], 'hi dad'.onig_regexp_split(OnigRegexp.new('\s*'))

  test_str = '1, 2, 3, 4, 5,, 6'
  assert_equal ['1', '2', '3', '4', '5', '', '6'], test_str.onig_regexp_split(OnigRegexp.new(',\s*'))

  test_str = '1,,2,3,,4,,'
  assert_equal ['1', '', '2', '3', '', '4'], test_str.onig_regexp_split(OnigRegexp.new(','))
  assert_equal ['1', '', '2', '3,,4,,'], test_str.onig_regexp_split(OnigRegexp.new(','), 4)
  assert_equal ['1', '', '2', '3', '', '4', '', ''], test_str.onig_regexp_split(OnigRegexp.new(','), -4)

  assert_equal [], ''.onig_regexp_split(OnigRegexp.new(','), -1)
end
