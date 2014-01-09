#include <stdio.h>
#include <memory.h>
#include <mruby.h>
#include <mruby/class.h>
#include <mruby/variable.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/data.h>
#include <mruby/variable.h>
#include "oniguruma.h"

static void
onig_regexp_free(mrb_state *mrb, void *p) {
  onig_free((OnigRegex) p);
}

static struct mrb_data_type mrb_onig_regexp_type = {
  "PosixRegexp", onig_regexp_free
};

static void
match_data_free(mrb_state* mrb, void* p) {
  (void)mrb;
  onig_region_free((OnigRegion*)p, 1);
}

static struct mrb_data_type mrb_onig_region_type = {
  "OnigRegion", match_data_free
};

static mrb_value
onig_regexp_initialize(mrb_state *mrb, mrb_value self) {
  mrb_value str, flag = mrb_nil_value();
  mrb_get_args(mrb, "S|o", &str, &flag);

  int cflag = 0;
  OnigSyntaxType* syntax = ONIG_SYNTAX_RUBY;
  if(mrb_nil_p(flag)) {
  } else if(mrb_type(flag) == MRB_TT_TRUE) {
    cflag |= ONIG_OPTION_IGNORECASE;
  } else if(mrb_fixnum_p(flag)) {
    int int_flags = mrb_fixnum(flag);
    if(int_flags & 0x1) { cflag |= ONIG_OPTION_IGNORECASE; }
    if(int_flags & 0x2) { cflag |= ONIG_OPTION_EXTEND; }
    if(int_flags & 0x4) { cflag |= ONIG_OPTION_MULTILINE; }
  } else if(mrb_string_p(flag)) {
    char const* str_flags = RSTRING_PTR(flag);
    if(strchr(str_flags, 'i')) { cflag |= ONIG_OPTION_IGNORECASE; }
    if(strchr(str_flags, 'x')) { cflag |= ONIG_OPTION_EXTEND; }
    if(strchr(str_flags, 'm')) { cflag |= ONIG_OPTION_MULTILINE; }
  } else {
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "unknown regexp flag: %S", flag);
  }

  OnigErrorInfo einfo;
  OnigRegex reg;
  if (onig_new(&reg, (OnigUChar*)RSTRING_PTR(str), (OnigUChar*) RSTRING_PTR(str) + RSTRING_LEN(str),
               cflag, ONIG_ENCODING_UTF8, syntax, &einfo) != ONIG_NORMAL) {
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "'%S' is an invalid regular expression because %S.",
               str, mrb_str_new(mrb, (char const*)einfo.par, einfo.par_end - einfo.par));
  }
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@source"), str);

  DATA_PTR(self) = reg;
  DATA_TYPE(self) = &mrb_onig_regexp_type;

  return self;
}

static mrb_value
onig_regexp_match(mrb_state *mrb, mrb_value self) {
  mrb_value str;
  OnigRegex reg;
  mrb_int pos = 0;

  mrb_get_args(mrb, "S|i", &str, &pos);
  if (pos < 0 || pos >= RSTRING_LEN(str)) {
    return mrb_nil_value();
  }

  Data_Get_Struct(mrb, self, &mrb_onig_regexp_type, reg);

  OnigRegion* match = onig_region_new();
  OnigUChar const* str_ptr = (OnigUChar*)RSTRING_PTR(str);
  int match_result = onig_search(reg, str_ptr, str_ptr + RSTRING_LEN(str),
                                 str_ptr + pos, str_ptr + RSTRING_LEN(str), match, 0);
  if (match_result == ONIG_MISMATCH) {
    onig_region_free(match, 0);
    return mrb_nil_value();
  } else if (match_result < 0) {
    char err[ONIG_MAX_ERROR_MESSAGE_LEN] = "";
    onig_error_code_to_str((OnigUChar*)err, match_result);
    mrb_raise(mrb, E_REGEXP_ERROR, err);
  }

  mrb_value c = mrb_obj_value(mrb_data_object_alloc(
      mrb, mrb_class_get(mrb, "OnigMatchData"), match, &mrb_onig_region_type));
  mrb_iv_set(mrb, c, mrb_intern_lit(mrb, "string"), mrb_str_dup(mrb, str));
  mrb_iv_set(mrb, c, mrb_intern_lit(mrb, "regexp"), self);

  mrb_obj_iv_set(mrb, (struct RObject *)mrb_class_real(RDATA(self)->c), mrb_intern_lit(mrb, "@last_match"), c);
  return c;
}

static mrb_value
onig_regexp_equal(mrb_state *mrb, mrb_value self) {
  mrb_value other;
  OnigRegex self_reg, other_reg;

  mrb_get_args(mrb, "o", &other);
  if (mrb_obj_equal(mrb, self, other)){
    return mrb_true_value();
  }
  if (mrb_nil_p(other)) {
    return mrb_false_value();
  }
  Data_Get_Struct(mrb, self, &mrb_onig_regexp_type, self_reg);
  Data_Get_Struct(mrb, other, &mrb_onig_regexp_type, other_reg);

  if (!self_reg || !other_reg){
      mrb_raise(mrb, E_RUNTIME_ERROR, "Invalid OnigRegexp");
  }
  if (onig_get_options(self_reg) != onig_get_options(other_reg)){
      return mrb_false_value();
  }
  return mrb_str_equal(mrb, mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")), mrb_iv_get(mrb, other, mrb_intern_lit(mrb, "@source"))) ?
      mrb_true_value() : mrb_false_value();
}

static mrb_value
onig_regexp_casefold_p(mrb_state *mrb, mrb_value self) {
  OnigRegex reg;

  Data_Get_Struct(mrb, self, &mrb_onig_regexp_type, reg);
  return (onig_get_options(reg) & ONIG_OPTION_IGNORECASE) ? mrb_true_value() : mrb_false_value();
}

static mrb_value
onig_regexp_version(mrb_state* mrb, mrb_value self) {
  (void)self;
  return mrb_str_new_cstr(mrb, onig_version());
}

static mrb_value
match_data_to_a(mrb_state* mrb, mrb_value self);

// ISO 15.2.16.3.1
static mrb_value
match_data_index(mrb_state* mrb, mrb_value self) {
  mrb_int idx;
  mrb_get_args(mrb, "i", &idx);
  mrb_value ary = match_data_to_a(mrb, self);
  return mrb_ary_entry(ary, idx);
}

#define match_data_check_index() \
  if(idx < 0 || reg->num_regs <= idx) \
    mrb_raisef(mrb, E_INDEX_ERROR, "index %S out of matches", mrb_fixnum_value(idx)) \

// ISO 15.2.16.3.2
static mrb_value
match_data_begin(mrb_state* mrb, mrb_value self) {
  mrb_int idx;
  mrb_get_args(mrb, "i", &idx);
  OnigRegion* reg;
  Data_Get_Struct(mrb, self, &mrb_onig_region_type, reg);
  match_data_check_index();
  return mrb_fixnum_value(reg->beg[idx]);
}

// ISO 15.2.16.3.3
static mrb_value
match_data_captures(mrb_state* mrb, mrb_value self) {
  mrb_value ary = match_data_to_a(mrb, self);
  return mrb_ary_new_from_values(mrb, RARRAY_LEN(ary) - 1, RARRAY_PTR(ary) + 1);
}

// ISO 15.2.16.3.4
static mrb_value
match_data_end(mrb_state* mrb, mrb_value self) {
  mrb_int idx;
  mrb_get_args(mrb, "i", &idx);
  OnigRegion* reg;
  Data_Get_Struct(mrb, self, &mrb_onig_region_type, reg);
  match_data_check_index();
  return mrb_fixnum_value(reg->end[idx]);
}

// ISO 15.2.16.3.5
static mrb_value
match_data_copy(mrb_state* mrb, mrb_value self) {
  mrb_value src_val;
  mrb_get_args(mrb, "o", &src_val);

  OnigRegion* src;
  Data_Get_Struct(mrb, src_val, &mrb_onig_region_type, src);

  OnigRegion* dst = onig_region_new();
  onig_region_copy(dst, src);

  DATA_PTR(self) = dst;
  DATA_TYPE(self) = &mrb_onig_region_type;
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "string"), mrb_iv_get(mrb, src_val, mrb_intern_lit(mrb, "string")));
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "regexp"), mrb_iv_get(mrb, src_val, mrb_intern_lit(mrb, "regexp")));
  return self;
}

// ISO 15.2.16.3.6
// ISO 15.2.16.3.10
static mrb_value
match_data_length(mrb_state* mrb, mrb_value self) {
  OnigRegion* reg;
  Data_Get_Struct(mrb, self, &mrb_onig_region_type, reg);
  return mrb_fixnum_value(reg->num_regs);
}

// ISO 15.2.16.3.7
static mrb_value
match_data_offset(mrb_state* mrb, mrb_value self) {
  mrb_int idx;
  mrb_get_args(mrb, "i", &idx);
  OnigRegion* reg;
  Data_Get_Struct(mrb, self, &mrb_onig_region_type, reg);
  match_data_check_index();
  mrb_value ret = mrb_ary_new_capa(mrb, 2);
  mrb_ary_push(mrb, ret, mrb_fixnum_value(reg->beg[idx]));
  mrb_ary_push(mrb, ret, mrb_fixnum_value(reg->end[idx]));
  return ret;
}

// ISO 15.2.16.3.8
static mrb_value
match_data_post_match(mrb_state* mrb, mrb_value self) {
  OnigRegion* reg;
  Data_Get_Struct(mrb, self, &mrb_onig_region_type, reg);
  mrb_value str = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "string"));
  return mrb_str_substr(mrb, str, reg->end[0], RSTRING_LEN(str) - reg->end[0]);
}

// ISO 15.2.16.3.9
static mrb_value
match_data_pre_match(mrb_state* mrb, mrb_value self) {
  OnigRegion* reg;
  Data_Get_Struct(mrb, self, &mrb_onig_region_type, reg);
  mrb_value str = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "string"));
  return mrb_str_substr(mrb, str, 0, reg->beg[0]);
}

// ISO 15.2.16.3.11
static mrb_value
match_data_string(mrb_state* mrb, mrb_value self) {
  return mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "string"));
}

static mrb_value
match_data_regexp(mrb_state* mrb, mrb_value self) {
  return mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "regexp"));
}

// ISO 15.2.16.3.12
static mrb_value
match_data_to_a(mrb_state* mrb, mrb_value self) {
  mrb_value cache = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "cache"));
  if(!mrb_nil_p(cache)) {
    return cache;
  }

  mrb_value str = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "string"));
  OnigRegion* reg;
  Data_Get_Struct(mrb, self, &mrb_onig_region_type, reg);

  mrb_value ret = mrb_ary_new_capa(mrb, reg->num_regs);
  int i, ai = mrb_gc_arena_save(mrb);
  for(i = 0; i < reg->num_regs; ++i) {
    mrb_ary_push(mrb, ret, mrb_str_substr(mrb, str, reg->beg[i], reg->end[i] - reg->beg[i]));
    mrb_gc_arena_restore(mrb, ai);
  }
  return ret;
}

// ISO 15.2.16.3.13
static mrb_value
match_data_to_s(mrb_state* mrb, mrb_value self) {
  mrb_value str = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "string"));
  OnigRegion* reg;
  Data_Get_Struct(mrb, self, &mrb_onig_region_type, reg);
  return mrb_str_substr(mrb, str, reg->beg[0], reg->end[0] - reg->beg[0]);
}

void
mrb_mruby_onig_regexp_gem_init(mrb_state* mrb) {
  struct RClass *clazz;

  clazz = mrb_define_class(mrb, "OnigRegexp", mrb->object_class);
  MRB_SET_INSTANCE_TT(clazz, MRB_TT_DATA);

  mrb_define_const(mrb, clazz, "IGNORECASE", mrb_fixnum_value(1));
  mrb_define_const(mrb, clazz, "EXTENDED", mrb_fixnum_value(2));
  mrb_define_const(mrb, clazz, "MULTILINE", mrb_fixnum_value(4));

  mrb_define_method(mrb, clazz, "initialize", onig_regexp_initialize, ARGS_REQ(1) | ARGS_OPT(2));
  mrb_define_method(mrb, clazz, "==", onig_regexp_equal, ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "match", onig_regexp_match, ARGS_REQ(1) | ARGS_OPT(1));
  mrb_define_method(mrb, clazz, "casefold?", onig_regexp_casefold_p, ARGS_NONE());

  mrb_define_module_function(mrb, clazz, "version", onig_regexp_version, MRB_ARGS_NONE());

  struct RClass* match_data = mrb_define_class(mrb, "OnigMatchData", mrb->object_class);
  MRB_SET_INSTANCE_TT(clazz, MRB_TT_DATA);
  mrb_undef_class_method(mrb, match_data, "new");

  // mrb_define_method(mrb, match_data, "==", &match_data_eq);
  mrb_define_method(mrb, match_data, "[]", &match_data_index, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, match_data, "begin", &match_data_begin, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, match_data, "captures", &match_data_captures, MRB_ARGS_NONE());
  mrb_define_method(mrb, match_data, "end", &match_data_end, MRB_ARGS_REQ(1));
  // mrb_define_method(mrb, match_data, "eql?", &match_data_eq);
  // mrb_define_method(mrb, match_data, "hash", &match_data_hash);
  mrb_define_method(mrb, match_data, "initialize_copy", &match_data_copy, MRB_ARGS_REQ(1));
  // mrb_define_method(mrb, match_data, "inspect", &match_data_inspect);
  mrb_define_method(mrb, match_data, "length", &match_data_length, MRB_ARGS_NONE());
  // mrb_define_method(mrb, match_data, "names", &match_data_names);
  mrb_define_method(mrb, match_data, "offset", &match_data_offset, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, match_data, "post_match", &match_data_post_match, MRB_ARGS_NONE());
  mrb_define_method(mrb, match_data, "pre_match", &match_data_pre_match, MRB_ARGS_NONE());
  mrb_define_method(mrb, match_data, "regexp", &match_data_regexp, MRB_ARGS_NONE());
  mrb_define_method(mrb, match_data, "size", &match_data_length, MRB_ARGS_NONE());
  mrb_define_method(mrb, match_data, "string", &match_data_string, MRB_ARGS_NONE());
  mrb_define_method(mrb, match_data, "to_a", &match_data_to_a, MRB_ARGS_NONE());
  mrb_define_method(mrb, match_data, "to_s", &match_data_to_s, MRB_ARGS_NONE());
  // mrb_define_method(mrb, match_data, "values_at", &match_data_values_at);
}

void
mrb_mruby_onig_regexp_gem_final(mrb_state* mrb) {
  (void)mrb;
}

// vim:set et:
