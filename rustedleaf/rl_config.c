#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#include "rl_defs.h"
#include "rl_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Read functions */


   
int rl_conf_read_str(void *dst, const char *val, const size_t strlen) {
    const char **dst_str = dst;
    char *ret = malloc(strlen + 1);
    if (!ret) {
        return RL_ENOMEM;
    }
    memcpy(ret, val, strlen + 1);
    *dst_str = ret;
    return 0;
}


/* Read unsigned integers */


int rl_conf_read_u8(void *dst, const char *val, const size_t strlen) {
    uint8_t *dst_ret = dst;
    char *test;
    const unsigned long ret = strtoul(val, &test, 0);
    if (test == val) {
        return RL_EINVAL;
    }
    if (errno || ret > UINT8_MAX) {
        return RL_ERANGE;
    }
    *dst_ret = (uint8_t) ret;
    return 0;
}

int rl_conf_read_u16(void *dst, const char *val, const size_t strlen) {
    uint16_t *dst_ret = dst;
    char *test;
    const unsigned long ret = strtoul(val, &test, 0);
    if (test == val) {
        return RL_EINVAL;
    }
    if (errno || ret > UINT16_MAX) {
        return RL_ERANGE;
    }
    *dst_ret = (uint16_t) ret;
    return 0;
}

int rl_conf_read_u32(void *dst, const char *val, const size_t strlen) {
    uint32_t *dst_ret = dst;
    char *test;
    const unsigned long ret = strtoul(val, &test, 0);
    if (test == val) {
        return RL_EINVAL;
    }
    if (errno || ret > UINT32_MAX) {
        return RL_ERANGE;
    }
    *dst_ret = (uint32_t) ret;
    return 0;
}

int rl_conf_read_u64(void *dst, const char *val, const size_t strlen) {
    uint64_t *dst_ret = dst;
    char *test;
    const unsigned long long int ret = strtoull(val, &test, 0);
    if (test == val) {
        return RL_EINVAL;
    }
    if (errno || ret > UINT64_MAX) {
        return RL_ERANGE;
    }
    *dst_ret = (uint64_t) ret;
    return 0;
}


/* Read signed integers */


int rl_conf_read_s8(void *dst, const char *val, const size_t strlen) {
    int8_t *dst_ret = dst;
    char *test;
    const long ret = strtol(val, &test, 0);
    if (test == val) {
        return RL_EINVAL;
    }
    if (errno || ret > INT8_MAX || ret < INT8_MIN) {
        return RL_ERANGE;
    }
    *dst_ret = (int8_t) ret;
    return 0;
}

int rl_conf_read_s16(void *dst, const char *val, const size_t strlen) {
    int16_t *dst_ret = dst;
    char *test;
    const long ret = strtol(val, &test, 0);
    if (test == val) {
        return RL_EINVAL;
    }
    if (errno || ret > INT16_MAX || ret < INT16_MIN) {
        return RL_ERANGE;
    }
    *dst_ret = (int16_t) ret;
    return 0;
}

int rl_conf_read_s32(void *dst, const char *val, const size_t strlen) {
    int32_t *dst_ret = dst;
    char *test;
    const long ret = strtol(val, &test, 0);
    if (test == val) {
        return RL_EINVAL;
    }
    if (errno || ret > INT32_MAX || ret < INT32_MIN) {
        return RL_ERANGE;
    }
    *dst_ret = (int32_t) ret;
    return 0;
}

int rl_conf_read_s64(void *dst, const char *val, const size_t strlen) {
    int64_t *dst_ret = dst;
    char *test;
    const signed long long int ret = strtoll(val, &test, 0);
    if (test == val) {
        return RL_EINVAL;
    }
    if (errno || ret > INT64_MAX || ret < INT64_MIN) {
        return RL_ERANGE;
    }
    *dst_ret = (int64_t) ret;
    return 0;
}


/* Read floats */


int rl_conf_read_flt(void *dst, const char *val, const size_t strlen) {
    float *dst_ret = dst;
    char *test;
    const float ret = strtof(val, &test);
    if (test == val || !isfinite(ret)) {
        return RL_EINVAL;
    }
    if (errno) {
        return RL_ERANGE;
    }
    *dst_ret = ret;
    return 0;
}

int rl_conf_read_dbl(void *dst, const char *val, const size_t strlen) {
    double *dst_ret = dst;
    char *test;
    const double ret = strtod(val, &test);
    if (test == val || !isfinite(ret)) {
        return RL_EINVAL;
    }
    if (errno) {
        return RL_ERANGE;
    }
    *dst_ret = ret;
    return 0;
}






int rl_read_config(void *dst, const char *file_name, const struct rl_config_member *members, const size_t nmembers, 
    void (*nomember_function)(const char *member_name, const char *member_value)) {
    FILE *fd;
    errno_t open_err = fopen_s(&fd, file_name, "rt");
    if (open_err) {
        errno = open_err;
        return RL_EOTHER;
    }
    
    char line_buffer[4096];
    size_t member_predict = 0; /* Use this to predict which member is next */
    for (;;) {
        const char *ret = fgets(line_buffer, (int) sizeof(line_buffer), fd);
        if (!ret) {
            int err = ferror(fd);
            fclose(fd);
            if (err) {
                errno = err;
                err = RL_EOTHER;
            }
            return err;
        }

        const size_t line_buffer_len = strlen(line_buffer);
        /* Min length */
        if (line_buffer_len < 3) {
            continue;
        }

        /* Comments */
        if (*line_buffer == '#') {
            continue;
        }
        

        char *splitter = memchr(line_buffer, '=', line_buffer_len);
        if (!splitter) {
            continue;
        }
        /* Set the splitter for the reader functions */
        *splitter = '\0';
        
        /* Find member responsible */
        const struct rl_config_member *current_member = members + member_predict;
        if (strcmp(line_buffer, current_member->name)) {
            /* Search through all members */
            current_member = NULL;
            for (size_t i = 0; i < nmembers; ++i) {
                if (i == member_predict) {
                    /* Already know this one is invalid */
                    continue;
                }
                if (!strcmp(line_buffer, (members + i)->name)) {
                    current_member = members + i;
                    break;
                }
            }
            if (!current_member) {
                /* No member found */
                if (nomember_function != NULL) {
                    nomember_function(line_buffer, splitter + 1);
                }
                continue;
            }
        }
        const size_t value_length = strlen(splitter + 1);
        const int err = current_member->read_function(current_member->struct_offset + ((char *)dst), splitter + 1, value_length);
        if (err) {
            fclose(fd);
            return err;
        }
        if (member_predict != (nmembers - 1)) {
            ++member_predict;
        }
        continue;
    }
}

#ifdef __cplusplus
}
#endif
