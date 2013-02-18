
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
  (reg === "http://example.com") == true and (reg === "htt://example.com") == false
end

# TODO =~

assert("OnigRegexp#casefold?") do
  reg1 = OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", OnigRegexp::MULTILINE)
  reg2 = OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", OnigRegexp::IGNORECASE | OnigRegexp::EXTENDED)
  reg3 = OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", OnigRegexp::MULTILINE | OnigRegexp::IGNORECASE)
  reg4 = OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  reg5 = OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", true)

  reg1.casefold? == false and reg2.casefold? == true and reg3.casefold? == true and
    reg4.casefold? == false and reg5.casefold? == true
end

assert("OnigRegexp#match") do
  reg = OnigRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  reg.match("http://masamitsu-murase.12345/hoge.html") and
    reg.match("http:///masamitsu-murase.12345/hoge.html").nil?
end

assert("OnigRegexp#source") do
  str = "(https?://[^/]+)[-a-zA-Z0-9./]+"
  reg = OnigRegexp.new(str)

  reg.source == str
end

# Extended patterns.
assert("OnigRegexp#match (no flags)") do
  patterns = [
    [ OnigRegexp.new(".*"), "abcd\nefg", "abcd" ],
    [ OnigRegexp.new("^a."), "abcd\naefg", "ab" ],
    [ OnigRegexp.new("^a."), "bacd\naefg", "ae" ],
    [ OnigRegexp.new(".$"), "bacd\naefg", "d" ]
  ]

  patterns.all?{ |reg, str, result| reg.match(str)[0] == result }
end

assert("OnigRegexp#match (multiline)") do
  patterns = [
    [ OnigRegexp.new(".*", OnigRegexp::MULTILINE), "abcd\nefg", "abcd\nefg" ]
  ]

  patterns.all?{ |reg, str, result| reg.match(str)[0] == result }
end

assert("OnigRegexp#match (ignorecase)") do
  patterns = [
    [ OnigRegexp.new("aBcD", OnigRegexp::IGNORECASE|OnigRegexp::EXTENDED), "00AbcDef", "AbcD" ],
    [ OnigRegexp.new("0x[a-f]+", OnigRegexp::IGNORECASE|OnigRegexp::EXTENDED), "00XaBCdefG", "0XaBCdef" ],
    [ OnigRegexp.new("0x[^c-f]+", OnigRegexp::IGNORECASE|OnigRegexp::EXTENDED), "00XaBCdefG", "0XaB" ]
  ]

  patterns.all?{ |reg, str, result| reg.match(str)[0] == result }
end

