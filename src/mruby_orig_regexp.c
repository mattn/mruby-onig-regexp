#include <stdio.h>
#include <memory.h>
#include <mruby.h>
#include <mruby/class.h>
#include <mruby/variable.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/data.h>
#include "onigposix.h"

struct mrb_regex_t {
  regex_t re;
};

static void
onig_regexp_free(mrb_state *mrb, void *p) {
  struct mrb_regex_t *pre = (struct mrb_regex_t *) p;
  regfree(&pre->re);
  mrb_free(mrb, pre);
}

static struct mrb_data_type mrb_regex_t_type = {
  "PosixRegexp", onig_regexp_free
};

static void
onig_regexp_init(mrb_state *mrb, mrb_value self, mrb_value str, mrb_value flag) {
  mrb_value regex;
  struct mrb_regex_t *reg;

  regex = mrb_iv_get(mrb, self, mrb_intern(mrb, "@regex"));
  if (mrb_nil_p(regex)) {
    reg = malloc(sizeof(struct mrb_regex_t));
    memset(reg, 0, sizeof(struct mrb_regex_t));
    mrb_iv_set(mrb, self, mrb_intern(mrb, "@regex"), mrb_obj_value(
        Data_Wrap_Struct(mrb, mrb->object_class,
          &mrb_regex_t_type, (void*) reg)));
  }else{
    Data_Get_Struct(mrb, regex, &mrb_regex_t_type, reg);
    regfree(&reg->re);
  }

  int regerr = regcomp(&reg->re, RSTRING_PTR(str), REG_EXTENDED);
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
  mrb_value source, flag;

  mrb_get_args(mrb, "S|S", &source, &flag);

  onig_regexp_init(mrb, self, source, flag);

  return mrb_nil_value();
}

static mrb_value
onig_regexp_match(mrb_state *mrb, mrb_value self) {
  const char *str;
  struct mrb_regex_t *reg;

  mrb_get_args(mrb, "z", &str);

  mrb_value regex;
  regex = mrb_iv_get(mrb, self, mrb_intern(mrb, "@regex"));
  Data_Get_Struct(mrb, regex, &mrb_regex_t_type, reg);

  int i;
  size_t nmatch = 999;
  regmatch_t match[nmatch];
  for (i = 0; i < nmatch; i++)
    match[i].rm_so = -1;
  int regerr = regexec(&reg->re, str, nmatch, match, 0);
  if (regerr) {
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

  mrb_iv_set(mrb, self, mrb_intern(mrb, "@last_match"), c);
  return c;
}

void
mrb_mruby_onig_regexp_gem_init(mrb_state* mrb) {
  struct RClass *clazz;

  clazz = mrb_define_class(mrb, "OnigRegexp", mrb->object_class);
  mrb_define_method(mrb, clazz, "initialize", onig_regexp_initialize, ARGS_REQ(1) | ARGS_OPT(2));
  mrb_define_method(mrb, clazz, "match", onig_regexp_match, ARGS_REQ(1));
}

void
mrb_mruby_onig_regexp_gem_final(mrb_state* mrb) {
}

// vim:set et:
