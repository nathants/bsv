// based on source: https://github.com/DataDog/sketches-go/tree/43f19ad77ff73bc865c41f7e391ef4b23b82810a/ddsketch
//
// Unless explicitly stated otherwise all files in this repository are licensed
// under the Apache License 2.0.
// This product includes software developed at Datadog (https://www.datadoghq.com/).
// Copyright 2018 Datadog, Inc.

#pragma once

#include <float.h>
#include <math.h>
#include "util.h"
#include "row.h"

// constants

#define DEFAULT_MAX_NUM_BINS 2048
#define DEFAULT_ALPHA 0.01
#define DEFAULT_MIN_VALUE 1.0e-9
#define INITIAL_BINS 128
#define GROW_LEFT_BY 128

// structs

typedef struct config_s {
    i32 max_num_bins;
    f64 gamma;
    f64 gamma_ln;
    f64 min_value;
    i32 offset;
} config_t;

typedef struct store_s {
    i64 *bins;
    i32  num_bins;
    i32  max_num_bins;
    i64  count;
    i32  min_key;
    i32  max_key;
} store_t;

typedef struct sketch_s {
    config_t *config;
    store_t  *store;
    f64       min;
    f64       max;
    i64       count;
    f64       sum;
} sketch_t;

// config

f64 config_log_gamma(config_t *c, f64 v) {
    return log(v) / c->gamma_ln;
}

f64 config_pow_gamma(config_t *c, i32 k) {
    return exp((f64)k * c->gamma_ln);
}

config_t *config_new(f64 alpha, i32 max_num_bins, f64 min_value) {
    config_t *c;
    MALLOC(c, sizeof(config_t));
    c->max_num_bins = max_num_bins;
    c->gamma = 1 + 2 * alpha / (1 - alpha);
    c->gamma_ln = log1p(2 * alpha / (1 - alpha));
    c->min_value = min_value;
    c->offset = -(i32)config_log_gamma(c, min_value) + 1;
    return c;
}

config_t *config_new_default() {
    return config_new(DEFAULT_ALPHA, DEFAULT_MAX_NUM_BINS, DEFAULT_MIN_VALUE);
}

i32 config_key(config_t *c, f64 v) {
    if (v < -c->min_value) {
        return -(i32)ceil(config_log_gamma(c, -v)) - c->offset;
    } else if (v > c->min_value) {
        return (i32)ceil(config_log_gamma(c, v)) + c->offset;
    } else {
        return 0;
    }
}

// store

store_t *store_new(i32 max_num_bins) {
    store_t *s;
    MALLOC(s, sizeof(store_t));
    memset(s, 0, sizeof(store_t));
    s->num_bins = INITIAL_BINS;
    MALLOC(s->bins,    s->num_bins * sizeof(i64));
    memset(s->bins, 0, s->num_bins * sizeof(i64));
    s->max_num_bins = max_num_bins;
    return s;
}

i32 store_key_at_rank(store_t *s, i32 rank) {
    i32 n = 0;
    for (i32 i = 0; i < s->num_bins; i++) {
        n += (i32)s->bins[i];
        if (n >= rank) {
            return i + s->min_key;
        }
    }
    return s->max_key;
}

void store_grow_left(store_t *s, i32 key) {
    if (s->min_key < key || s->num_bins >= s->max_num_bins) {
        return;
    }
    i32 min_key;
    if (s->max_key - key >= s->max_num_bins) {
        min_key = s->max_key - s->max_num_bins + 1;
    } else {
        min_key = s->min_key;
        while (min_key > key) {
            min_key -= GROW_LEFT_BY;
        }
    }
    i64 *tmp_bins;
    i32 num_bins = s->max_key - min_key + 1;
    i32 offset = s->min_key - min_key;
    MALLOC(tmp_bins,    num_bins * sizeof(i64));
    memset(tmp_bins, 0, num_bins * sizeof(i64));
    ASSERT(num_bins >= s->num_bins, "fatal: overflow 1\n");
    ASSERT(num_bins - offset >= s->num_bins, "fatal overflow 1b\n");
    memcpy(tmp_bins + offset, s->bins, s->num_bins * sizeof(i64));
    free(s->bins);
    s->bins = tmp_bins;
    s->num_bins = num_bins;
    s->min_key = min_key;
}

void store_grow_right(store_t *s, i32 key) {
    if (s->max_key > key) {
        return;
    }
    if (key - s->max_key >= s->max_num_bins) {
        i64 *tmp_bins;
        MALLOC(tmp_bins,    s->max_num_bins * sizeof(i64));
        memset(tmp_bins, 0, s->max_num_bins * sizeof(i64));
        tmp_bins[0] = (i64)s->count;
        free(s->bins);
        s->bins = tmp_bins;
        s->num_bins = s->max_num_bins;
        s->max_key = key;
        s->min_key = key - s->max_num_bins + 1;
    } else if (key - s->min_key >= s->max_num_bins) {
        i32 min_key = key - s->max_num_bins + 1;
        i64 n = 0;
        for (i32 i = s->min_key; i < min_key && i <= s->max_key; i++) {
            ASSERT(i - s->min_key < s->num_bins, "fatal: index error 2\n");
            n += s->bins[i - s->min_key];
        }
        if (s->num_bins < s->max_num_bins) {
            i64 *tmp_bins;
            MALLOC(tmp_bins,    s->max_num_bins * sizeof(i64));
            memset(tmp_bins, 0, s->max_num_bins * sizeof(i64));
            i32 offset = min_key - s->min_key;
            ASSERT(s->max_num_bins >= s->num_bins - offset, "fatal: overflow 3\n");
            memcpy(tmp_bins, s->bins + offset, (s->num_bins - offset) * sizeof(i64));
            free(s->bins);
            s->bins = tmp_bins;
            s->num_bins = s->max_num_bins;
        } else {
            i32 offset = min_key - s->min_key;
            ASSERT(offset > 0, "fatal: bad offset 4\n");
            memmove(s->bins, s->bins + offset, (s->num_bins - offset) * sizeof(i64));
            for (i32 i = s->max_key - min_key + 1; i < s->max_num_bins; i++) {
                ASSERT(i < s->num_bins, "fatal: index error 5\n");
                s->bins[i] = 0;
            }
        }
        s->max_key = key;
        s->min_key = min_key;
        ASSERT(s->num_bins > 0, "fatal: index error 6\n");
        s->bins[0] += n;
    } else {
        i32 num_bins = key - s->min_key + 1;
        i64 *tmp_bins;
        MALLOC(tmp_bins,    num_bins * sizeof(i64));
        memset(tmp_bins, 0, num_bins * sizeof(i64));
        ASSERT(num_bins >= s->num_bins, "fatal: overflow 7\n");
        memcpy(tmp_bins, s->bins, s->num_bins * sizeof(i64));
        free(s->bins);
        s->bins = tmp_bins;
        s->num_bins = num_bins;
        s->max_key = key;
    }
}

void store_copy(store_t *s, store_t *o) {
        i64 *tmp_bins;
        MALLOC(tmp_bins,    o->num_bins * sizeof(i64));
        memset(tmp_bins, 0, o->num_bins * sizeof(i64));
        memcpy(tmp_bins, o->bins, o->num_bins * sizeof(i64));
        free(s->bins);
        s->bins = tmp_bins;
        s->num_bins = o->num_bins;
        s->max_num_bins = o->max_num_bins;
        s->count = o->count;
        s->min_key = o->min_key;
        s->max_key = o->max_key;
}

void store_merge(store_t *s, store_t *o) {
    if (o->count == 0) {
        return;
    }
    if (s->count == 0) {
        store_copy(s, o);
        return;
    }
    if (s->max_key > o->max_key) {
        if (o->min_key < s->min_key) {
            store_grow_left(s, o->min_key);
        }
        for (i32 i = MAX(o->min_key, s->min_key); i <= o->max_key; i++) {
            ASSERT(i - s->min_key < s->num_bins, "fatal: index error 8\n");
            ASSERT(i - o->min_key < o->num_bins, "fatal: index error 9\n");
            s->bins[i - s->min_key] += o->bins[i - o->min_key];
        }
        i64 n = 0;
        for (i32 i = o->min_key; i < s->min_key; i++) {
            ASSERT(i - o->min_key < o->num_bins, "fatal: index error 10\n");
            n += o->bins[i - o->min_key];
        }
        ASSERT(s->num_bins > 0, "fatal: index error 11\n");
        s->bins[0] += n;
    } else {
        if (o->min_key < s->min_key) {
            i64 *tmp_bins;
            MALLOC(tmp_bins,    o->num_bins * sizeof(i64));
            memset(tmp_bins, 0, o->num_bins * sizeof(i64));
            memcpy(tmp_bins, o->bins, o->num_bins * sizeof(i64));
            for (i32 i = s->min_key; i <= s->max_key; i++) {
                ASSERT(i - o->min_key < o->num_bins, "fatal: index error 12\n");
                ASSERT(i - s->min_key < s->num_bins, "fatal: index error 13\n");
                tmp_bins[i - o->min_key] += s->bins[i - s->min_key];
            }
            free(s->bins);
            s->bins = tmp_bins;
            s->num_bins = o->num_bins;
            s->min_key = o->min_key;
            s->max_key = o->max_key;
        } else {
            store_grow_right(s, o->max_key);
            for (i32 i = o->min_key; i <= o->max_key; i++) {
                ASSERT(i - s->min_key < s->num_bins, "fatal: index error 14\n");
                ASSERT(i - o->min_key < o->num_bins, "fatal: index error 15\n");
                s->bins[i - s->min_key] += o->bins[i - o->min_key];
            }
        }
    }
    s->count += o->count;
}

void store_add(store_t *s, i32 key) {
    if (s->count == 0) {
        s->max_key = key;
        s->min_key = key - s->num_bins + 1;
    }
    if (key < s->min_key) {
        store_grow_left(s, key);
    } else if (key > s->max_key) {
        store_grow_right(s, key);
    }
    i32 index = key - s->min_key;
    if (index < 0) {
        index = 0;
    }
    ASSERT(index < s->num_bins, "fatal: index error 16\n");
    s->bins[index]++;
    s->count++;
}

// sketch

sketch_t *sketch_new(config_t *c) {
    sketch_t *s;
    MALLOC(s, sizeof(sketch_t));
    memset(s, 0, sizeof(sketch_t));
    s->config = c;
    s->store = store_new(c->max_num_bins);
    s->min = DBL_MAX;
    s->max = DBL_MIN;
    return s;
}

void sketch_add(sketch_t *s, f64 v) {
    i32 key = config_key(s->config, v);
    store_add(s->store, key);
    if (v < s->min) {
        s->min = v;
    }
    if (s->max < v) {
        s->max = v;
    }
    s->count++;
    s->sum += v;
}

f64 sketch_quantile(sketch_t *s, f64 q) {
    ASSERT(q >= 0 && q <= 1 && s->count != 0, "fatal: bad q param or no data 17\n");
    if (q == 0) {
        return s->min;
    } else if (q == 1) {
        return s->max;
    }
    i32 rank = (q * (f64)(s->count - 1) + 1);
    i32 key = store_key_at_rank(s->store, rank);
    f64 quantile;
    if (key < 0) {
        key += s->config->offset;
        quantile = -2 * config_pow_gamma(s->config, -key) / (1 + s->config->gamma);
    } else if (key > 0) {
        key -= s->config->offset;
        quantile = 2 * config_pow_gamma(s->config, key) / (1 + s->config->gamma);
    } else {
        quantile = 0;
    }
    if (quantile < s->min) {
        quantile = s->min;
    }
    if (quantile > s->max) {
        quantile = s->max;
    }
    return quantile;
}

void sketch_merge(sketch_t *s, sketch_t *o) {
    if (o->count == 0) {
        return;
    }
    if (s->count == 0) {
        store_copy(s->store, o->store);
        s->count = o->count;
        s->sum = o->sum;
        s->min = o->min;
        s->max = o->max;
        return;
    }
    store_merge(s->store, o->store);
    s->count += o->count;
    s->sum += o->sum;
    if (o->min < s->min) {
        s->min = o->min;
    }
    if (o->max > s->max) {
        s->max = o->max;
    }
}

// serialization to/from bsv rows. not based on datadog source

void sketch_to_row(row_t *row, sketch_t *s) {
    // sketch_t
    row->columns[0] = &s->min;   row->sizes[0] = sizeof(f64);
    row->columns[1] = &s->max;   row->sizes[1] = sizeof(f64);
    row->columns[2] = &s->count; row->sizes[2] = sizeof(i64);
    row->columns[3] = &s->sum;   row->sizes[3] = sizeof(f64);
    // config_t
    row->columns[4] = &s->config->max_num_bins; row->sizes[4] = sizeof(i32);
    row->columns[5] = &s->config->gamma;        row->sizes[5] = sizeof(f64);
    row->columns[6] = &s->config->gamma_ln;     row->sizes[6] = sizeof(f64);
    row->columns[7] = &s->config->min_value;    row->sizes[7] = sizeof(f64);
    row->columns[8] = &s->config->offset;       row->sizes[8] = sizeof(i32);
    // store_t
    row->columns[9]  = &s->store->num_bins;     row->sizes[9]  = sizeof(i32);
    row->columns[10] = &s->store->max_num_bins; row->sizes[10] = sizeof(i32);
    row->columns[11] = &s->store->count;        row->sizes[11] = sizeof(i64);
    row->columns[12] = &s->store->min_key;      row->sizes[12] = sizeof(i32);
    row->columns[13] = &s->store->max_key;      row->sizes[13] = sizeof(i32);
    // store_t.bins
    i32 offset = 14;
    for (i32 i = 0; i < s->store->num_bins; i++) {
        row->columns[i + offset] = &s->store->bins[i];
        row->sizes[i + offset] = sizeof(i64);
        row->max = offset + i;
    }
}

sketch_t *sketch_from_row(row_t *row) {
    config_t *c = config_new_default();
    sketch_t *s = sketch_new(c);
    ASSERT(row->max >= 14, "fatal: not enough data 18\n");
    // sketch_t
    s->min   = *(f64*)row->columns[0]; ASSERT(row->sizes[0] == sizeof(f64), "fatal: bad size 19\n");
    s->max   = *(f64*)row->columns[1]; ASSERT(row->sizes[1] == sizeof(f64), "fatal: bad size 20\n");
    s->count = *(i64*)row->columns[2]; ASSERT(row->sizes[2] == sizeof(i64), "fatal: bad size 21\n");
    s->sum   = *(f64*)row->columns[3]; ASSERT(row->sizes[3] == sizeof(f64), "fatal: bad size 22\n");
    // config_t
    s->config->max_num_bins = *(i32*)row->columns[4]; ASSERT(row->sizes[4] == sizeof(i32), "fatal: bad size 23\n");
    s->config->gamma        = *(f64*)row->columns[5]; ASSERT(row->sizes[5] == sizeof(f64), "fatal: bad size 24\n");
    s->config->gamma_ln     = *(f64*)row->columns[6]; ASSERT(row->sizes[6] == sizeof(f64), "fatal: bad size 25\n");
    s->config->min_value    = *(f64*)row->columns[7]; ASSERT(row->sizes[7] == sizeof(f64), "fatal: bad size 26\n");
    s->config->offset       = *(i32*)row->columns[8]; ASSERT(row->sizes[8] == sizeof(i32), "fatal: bad size 27\n");
    // store_t
    s->store->num_bins     = *(i32*)row->columns[9];  ASSERT(row->sizes[9]  == sizeof(i32), "fatal: bad size 28\n");
    s->store->max_num_bins = *(i32*)row->columns[10]; ASSERT(row->sizes[10] == sizeof(i32), "fatal: bad size 29\n");
    s->store->count        = *(i64*)row->columns[11]; ASSERT(row->sizes[11] == sizeof(i64), "fatal: bad size 30\n");
    s->store->min_key      = *(i32*)row->columns[12]; ASSERT(row->sizes[12] == sizeof(i32), "fatal: bad size 31\n");
    s->store->max_key      = *(i32*)row->columns[13]; ASSERT(row->sizes[13] == sizeof(i32), "fatal: bad size 32\n");
    // store_t.bins
    ASSERT(row->max == 14 + s->store->num_bins - 1, "fatal: not enough bin data [%d %d] 34\n");
    free(s->store->bins);
    MALLOC(s->store->bins,    s->store->num_bins * sizeof(i64));
    memset(s->store->bins, 0, s->store->num_bins * sizeof(i64));
    i32 offset = 14;
    for (i32 i = 0; i < s->store->num_bins; i++) {
        s->store->bins[i] = *(i64*)row->columns[i + offset];
        ASSERT(row->sizes[i + offset] == sizeof(i64), "fatal: bad size 35\n");
    }
    return s;
}
