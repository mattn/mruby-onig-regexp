#include <stdio.h>
#include <memory.h>
#include <mruby.h>
#include <mruby/class.h>
#include <mruby/variable.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/data.h>
#include "onigposix.h"

struct mrb_onig_regexp {
  regex_t re;
  int flag;
};

static void
onig_regexp_free(mrb_state *mrb, void *p) {
  struct mrb_onig_regexp *pre = (struct mrb_onig_regexp *) p;
  regfree(&pre->re);
  mrb_free(mrb, pre);
}

static struct mrb_data_type mrb_onig_regexp_type = {
  "PosixRegexp", onig_regexp_free
};

static void
onig_regexp_init(mrb_state *mrb, mrb_value self, mrb_value str, mrb_value flag) {
  mrb_value regexp;
  struct mrb_onig_regexp *reg;

  regexp = mrb_iv_get(mrb, self, mrb_intern(mrb, "@regexp"));
  if (mrb_nil_p(regexp)) {
    reg = malloc(sizeof(struct mrb_onig_regexp));
    memset(reg, 0, sizeof(struct mrb_onig_regexp));
    mrb_iv_set(mrb, self, mrb_intern(mrb, "@regexp"), mrb_obj_value(
        Data_Wrap_Struct(mrb, mrb->object_class,
          &mrb_onig_regexp_type, (void*) reg)));
  }else{
    Data_Get_Struct(mrb, regexp, &mrb_onig_regexp_type, reg);
    regfree(&reg->re);
  }

  int cflag = 0;
  if (mrb_nil_p(flag))
    cflag = REG_EXTENDED | REG_NEWLINE;
  else if (mrb_type(flag) == MRB_TT_TRUE)
    cflag |= REG_ICASE;
  else if (mrb_string_p(flag)) {
    if (strchr(RSTRING_PTR(flag), 'i')) cflag |= REG_ICASE;
    if (strchr(RSTRING_PTR(flag), 'x')) cflag |= REG_EXTENDED;
    if (strchr(RSTRING_PTR(flag), 'm')) cflag |= REG_NEWLINE;
  }
  reg->flag = cflag;
  int regerr = regcomp(&reg->re, RSTRING_PTR(str), cflag);
  if (regerr) {
    char err[256];
    regerror(regerr, &reg->re, err, sizeof(err));
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "'%s' is an invalid regular expression because %s.",
      RSTRING_PTR(str), err);
  }
  mrb_iv_set(mrb, self, mrb_intern(mrb, "@source"), str);
}

static mrb_value
onig_regexp_initialize(mrb_state *mrb, mrb_value self) {
  mrb_value source, flag = mrb_nil_value();

  mrb_get_args(mrb, "S|o", &source, &flag);
  onig_regexp_init(mrb, self, source, flag);
  return mrb_nil_value();
}

static mrb_value
onig_regexp_match(mrb_state *mrb, mrb_value self) {
  const char *str;
  mrb_value regexp;
  struct mrb_onig_regexp *reg;

  mrb_get_args(mrb, "z", &str);

  regexp = mrb_iv_get(mrb, self, mrb_intern(mrb, "@regexp"));
  Data_Get_Struct(mrb, regexp, &mrb_onig_regexp_type, reg);

  int i;
  size_t nmatch = 999;
  regmatch_t match[nmatch];
  for (i = 0; i < nmatch; i++)
    match[i].rm_so = -1;
  int regerr = regexec(&reg->re, str, nmatch, match, 0);
  if (regerr == REG_NOMATCH)
    return mrb_nil_value();

  if (regerr != 0) {
    char err[256];
    regerror(regerr, &reg->re, err, sizeof(err));
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "'%s' is an invalid regular expression because %s.",
      str, err);
  }

  int ai = mrb_gc_arena_save(mrb);
  struct RClass* clazz;
  clazz = mrb_class_get(mrb, "OnigMatchData");
  mrb_value c = mrb_class_new_instance(mrb, 0, NULL, clazz);
  mrb_value args[2];
  for (i = 0; i < nmatch; i++) {
    if (match[i].rm_so != -1) {
      args[0] = mrb_fixnum_value(match[i].rm_so);
      args[1] = mrb_fixnum_value(match[i].rm_eo - match[i].rm_so);
      mrb_funcall_argv(mrb, c, mrb_intern(mrb, "push"), sizeof(args)/sizeof(args[0]), &args[0]);
      mrb_gc_arena_restore(mrb, ai);
    }
  }

  mrb_iv_set(mrb, c, mrb_intern(mrb, "@string"), mrb_str_new_cstr(mrb, str));
  mrb_iv_set(mrb, self, mrb_intern(mrb, "@last_match"), c);
  return c;
}

static mrb_value
onig_regexp_equal(mrb_state *mrb, mrb_value self) {
  mrb_value other, regexp_self, regexp_other;
  struct mrb_onig_regexp *self_reg, *other_reg;

  mrb_get_args(mrb, "o", &other);
  if (mrb_obj_equal(mrb, self, other)){
      return mrb_true_value();
  }
  regexp_self = mrb_iv_get(mrb, self, mrb_intern(mrb, "@regexp"));
  regexp_other = mrb_iv_get(mrb, other, mrb_intern(mrb, "@regexp"));
  Data_Get_Struct(mrb, regexp_self, &mrb_onig_regexp_type, self_reg);
  Data_Get_Struct(mrb, regexp_other, &mrb_onig_regexp_type, other_reg);

  if (!self_reg || !other_reg){
      mrb_raise(mrb, E_RUNTIME_ERROR, "Invalid OnigRegexp");
  }
  if (self_reg->flag != other_reg->flag){
      return mrb_false_value();
  }
  return mrb_str_equal(mrb, mrb_iv_get(mrb, self, mrb_intern(mrb, "@source")), mrb_iv_get(mrb, other, mrb_intern(mrb, "@source"))) ?
      mrb_true_value() : mrb_false_value();
}

static mrb_value
onig_regexp_casefold_p(mrb_state *mrb, mrb_value self) {
  mrb_value regexp;
  struct mrb_onig_regexp *reg;

  regexp = mrb_iv_get(mrb, self, mrb_intern(mrb, "@regexp"));
  Data_Get_Struct(mrb, regexp, &mrb_onig_regexp_type, reg);
  return (reg->flag & REG_ICASE) ? mrb_true_value() : mrb_false_value();
}

void
mrb_mruby_onig_regexp_gem_init(mrb_state* mrb) {
  struct RClass *clazz;

  clazz = mrb_define_class(mrb, "OnigRegexp", mrb->object_class);

  mrb_define_const(mrb, clazz, "IGNORECASE", mrb_fixnum_value(REG_ICASE));
  mrb_define_const(mrb, clazz, "EXTENDED", mrb_fixnum_value(REG_EXTENDED));
  mrb_define_const(mrb, clazz, "MULTILINE", mrb_fixnum_value(REG_NEWLINE));

  mrb_define_method(mrb, clazz, "initialize", onig_regexp_initialize, ARGS_REQ(1) | ARGS_OPT(2));
  mrb_define_method(mrb, clazz, "==", onig_regexp_equal, ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "match", onig_regexp_match, ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "casefold?", onig_regexp_casefold_p, ARGS_NONE());
}

void
mrb_mruby_onig_regexp_gem_final(mrb_state* mrb) {
}

// vim:set et:
