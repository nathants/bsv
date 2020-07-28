// source: https://github.com/DataDog/sketches-go/tree/43f19ad77ff73bc865c41f7e391ef4b23b82810a/ddsketch
// Unless explicitly stated otherwise all files in this repository are licensed
// under the Apache License 2.0.
// This product includes software developed at Datadog (https://www.datadoghq.com/).
// Copyright 2018 Datadog, Inc.

#pragma once

#include <float.h>
#include <math.h>
#include "util.h"

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

i32 store_length(store_t *s) {
    return s->num_bins;
}

i32 store_key_at_rank(store_t *s, i32 rank) {
    i32 n = 0;
    for (i32 i = 0; i < s->num_bins; i++) {
        n += (i32)s->bins[i] ;
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
    MALLOC(tmp_bins,    num_bins * sizeof(i64));
    memset(tmp_bins, 0, num_bins * sizeof(i64));
    memcpy(tmp_bins + s->min_key - min_key, s->bins, s->num_bins);
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
            n += s->bins[i - s->min_key];
        }
        if (s->num_bins < s->max_num_bins) {
            i64 *tmp_bins;
            MALLOC(tmp_bins,    s->max_num_bins * sizeof(i64));
            memset(tmp_bins, 0, s->max_num_bins * sizeof(i64));
            i32 offset = min_key - s->min_key;
            memcpy(tmp_bins, s->bins + offset, s->num_bins - offset);
            free(s->bins);
            s->bins = tmp_bins;
            s->num_bins = s->max_num_bins;
        } else {
            i32 offset = min_key - s->min_key;
            memmove(s->bins, s->bins + offset, s->num_bins - offset);
            for (i32 i = s->max_key - min_key + 1; i < s->max_num_bins; i++) {
                s->bins[i] = 0;
            }
        }
        s->max_key = key;
        s->min_key = min_key;
        s->bins[0] += n;
    } else {
        i32 num_bins = key - s->min_key + 1;
        i64 *tmp_bins;
        MALLOC(tmp_bins,    num_bins * sizeof(i64));
        memset(tmp_bins, 0, num_bins * sizeof(i64));
        memcpy(tmp_bins, s->bins, s->num_bins);
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
        memcpy(tmp_bins, o->bins, o->num_bins);
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
            s->bins[i - s->min_key] += o->bins[i - o->min_key];
        }
        i64 n = 0;
        for (i32 i = o->min_key; i < s->min_key; i++) {
            n += o->bins[i - o->min_key];
        }
        s->bins[0] += n;
    } else {
        if (o->min_key < s->min_key) {
            i64 *tmp_bins;
            MALLOC(tmp_bins,    o->num_bins * sizeof(i64));
            memset(tmp_bins, 0, o->num_bins * sizeof(i64));
            memcpy(tmp_bins, o->bins, o->num_bins);
            for (i32 i = s->min_key; i <= s->max_key; i++) {
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
    ASSERT(q >= 0 && q <= 1 && s->count != 0, "fatal: bad q param or no data\n");
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
