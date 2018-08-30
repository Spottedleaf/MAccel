#ifndef RL_CONFIG_H
#define RL_CONFIG_H RL_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

int rl_conf_read_str(void *dst, const char *val, const size_t strlen);

int rl_conf_read_u8 (void *dst, const char *val, const size_t strlen);
int rl_conf_read_u16(void *dst, const char *val, const size_t strlen);
int rl_conf_read_u32(void *dst, const char *val, const size_t strlen);
int rl_conf_read_u64(void *dst, const char *val, const size_t strlen);

int rl_conf_read_s8 (void *dst, const char *val, const size_t strlen);
int rl_conf_read_s16(void *dst, const char *val, const size_t strlen);
int rl_conf_read_s32(void *dst, const char *val, const size_t strlen);
int rl_conf_read_s64(void *dst, const char *val, const size_t strlen);

int rl_conf_read_flt(void *dst, const char *val, const size_t strlen);
int rl_conf_read_dbl(void *dst, const char *val, const size_t strlen);

struct rl_config_member {
    const char *name;
    size_t struct_offset;
    int (*read_function)(void *dst, const char *val, size_t strlen);
};

int rl_read_config(void *dst, const char *file_name, const struct rl_config_member *members, const size_t nmembers,
    void(*nomember_function)(const char *member_name, const char *member_value));

#define rl_create_confmem(n, s, mem, r_function) (struct rl_config_member) { .name = n, .struct_offset = offsetof(s, mem), \
    .read_function = r_function}

#define rl_create_confmem_str(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_str)

#define rl_create_confmem_u8(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_u8)
#define rl_create_confmem_u16(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_u16)
#define rl_create_confmem_u32(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_u32)
#define rl_create_confmem_u64(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_u64)

#define rl_create_confmem_s8(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_s8)
#define rl_create_confmem_s16(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_s16)
#define rl_create_confmem_s32(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_s32)
#define rl_create_confmem_s64(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_s64)

#define rl_create_confmem_flt(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_flt)
#define rl_create_confmem_dbl(n, s, mem) rl_create_confmem(n, s, mem, &rl_conf_read_dbl)


#ifdef __cplusplus
}
#endif

#endif /* RL_CONFIG_H */
