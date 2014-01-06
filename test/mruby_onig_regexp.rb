
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
