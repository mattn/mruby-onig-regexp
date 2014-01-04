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
onig_regexp_init(mrb_state *mrb, mrb_value self, mrb_value str, mrb_value flag) {
  if (!mrb_nil_p(mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, "@regexp")))) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "regexp already specified");
  }

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
  mrb_iv_set(mrb, self, mrb_intern_cstr(mrb, "@source"), str);

  mrb_iv_set(mrb, self, mrb_intern_cstr(mrb, "@regexp"), mrb_obj_value(
      Data_Wrap_Struct(mrb, mrb->object_class,
                       &mrb_onig_regexp_type, reg)));
}

static mrb_value
onig_regexp_initialize(mrb_state *mrb, mrb_value self) {
  mrb_value source, flag = mrb_nil_value();

  mrb_get_args(mrb, "S|o", &source, &flag);
  onig_regexp_init(mrb, self, source, flag);
  return mrb_nil_value();
}

struct traverse_arg {
  mrb_state* mrb;
  mrb_value result;
  int arena_idx;
};

static mrb_value
onig_regexp_match(mrb_state *mrb, mrb_value self) {
  mrb_value str;
  mrb_value regexp;
  OnigRegex reg;
  mrb_int pos = 0;

  mrb_get_args(mrb, "S|i", &str, &pos);
  if (pos < 0 || pos >= RSTRING_LEN(str)) {
    return mrb_nil_value();
  }

  regexp = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@regexp"));
  Data_Get_Struct(mrb, regexp, &mrb_onig_regexp_type, reg);

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

  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@last_match"), mrb_nil_value());

  mrb_value c = mrb_obj_new(mrb, mrb_class_get(mrb, "OnigMatchData"), 0, NULL);
  mrb_iv_set(mrb, c, mrb_intern_lit(mrb, "@string"), mrb_str_dup(mrb, str));

  int ai = mrb_gc_arena_save(mrb);
  int i;
  for(i = 0; i < match->num_regs; ++i) {
    mrb_value func_args[] = { mrb_fixnum_value(match->beg[i]), mrb_fixnum_value(match->end[i] - match->beg[i]) };
    mrb_funcall_argv(mrb, c, mrb_intern_lit(mrb, "push"), 2, func_args);
    mrb_gc_arena_restore(mrb, ai);
  }

  mrb_iv_set(mrb, self, mrb_intern_cstr(mrb, "@last_match"), c);
  onig_region_free(match, 0);
  return c;
}

static mrb_value
onig_regexp_equal(mrb_state *mrb, mrb_value self) {
  mrb_value other, regexp_self, regexp_other;
  OnigRegex self_reg, other_reg;

  mrb_get_args(mrb, "o", &other);
  if (mrb_obj_equal(mrb, self, other)){
    return mrb_true_value();
  }
  if (mrb_nil_p(other)) {
    return mrb_false_value();
  }
  regexp_self = mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, "@regexp"));
  regexp_other = mrb_iv_get(mrb, other, mrb_intern_cstr(mrb, "@regexp"));
  Data_Get_Struct(mrb, regexp_self, &mrb_onig_regexp_type, self_reg);
  Data_Get_Struct(mrb, regexp_other, &mrb_onig_regexp_type, other_reg);

  if (!self_reg || !other_reg){
      mrb_raise(mrb, E_RUNTIME_ERROR, "Invalid OnigRegexp");
  }
  if (onig_get_options(self_reg) != onig_get_options(other_reg)){
      return mrb_false_value();
  }
  return mrb_str_equal(mrb, mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, "@source")), mrb_iv_get(mrb, other, mrb_intern_cstr(mrb, "@source"))) ?
      mrb_true_value() : mrb_false_value();
}

static mrb_value
onig_regexp_casefold_p(mrb_state *mrb, mrb_value self) {
  mrb_value regexp;
  OnigRegex reg;

  regexp = mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, "@regexp"));
  Data_Get_Struct(mrb, regexp, &mrb_onig_regexp_type, reg);
  return (onig_get_options(reg) & ONIG_OPTION_IGNORECASE) ? mrb_true_value() : mrb_false_value();
}

void
mrb_mruby_onig_regexp_gem_init(mrb_state* mrb) {
  struct RClass *clazz;

  clazz = mrb_define_class(mrb, "OnigRegexp", mrb->object_class);

  mrb_define_const(mrb, clazz, "IGNORECASE", mrb_fixnum_value(1));
  mrb_define_const(mrb, clazz, "EXTENDED", mrb_fixnum_value(2));
  mrb_define_const(mrb, clazz, "MULTILINE", mrb_fixnum_value(4));

  mrb_define_method(mrb, clazz, "initialize", onig_regexp_initialize, ARGS_REQ(1) | ARGS_OPT(2));
  mrb_define_method(mrb, clazz, "==", onig_regexp_equal, ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "match", onig_regexp_match, ARGS_REQ(1) | ARGS_OPT(1));
  mrb_define_method(mrb, clazz, "casefold?", onig_regexp_casefold_p, ARGS_NONE());
}

void
mrb_mruby_onig_regexp_gem_final(mrb_state* mrb) {
  (void)mrb;
}

// vim:set et:
