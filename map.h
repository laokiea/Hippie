#ifndef _MAP_H_
#define _MAP_H_

#include "network.h"

#define MAX_HASH_TABLE_SIZE           100
#define HT_RESIZE_FACTORY             1.5f

#define HT_GET_INDEX(key_hash, hashtable) \
    (key_hash & hashtable->ht_mask) \

#define HT_BUCKET_INIT(ht,k,kh,v,s,t) do { \
    ht->key        = k; \
    ht->key_hash   = kh; \
    HERMES_VAL_COPY(ht, v, s, t); \
    ht->next       = NULL; \
    ht->index_next = NULL; \
} while(0) \

#define HERMES_VAL_COPY(b,v,s,t) do { \
    b->value = (hemers_hash_table_val_t *) malloc(sizeof(hemers_hash_table_val_t)); \
    memset(b->value, 0, sizeof(hemers_hash_table_val_t)); \
    b->value->val_size = s; \
    b->value->val_type = t; \
    b->value->val = malloc(s); \
    memcpy(b->value->val, v, s); \
} while(0) \

#define HVAL_PRINT(r) do { \
    switch (r->val_type) { \
        case H_VAL_TYPE_UINT16: \
        printf("%hu", *((uint16_t *)r->val)); \
        break; \
        case H_VAL_TYPE_UINT32: \
        printf("%u", *((uint32_t *)r->val)); \
        break; \
        case H_VAL_TYPE_UINT64: \
        printf("%llu", *((uint64_t *)r->val)); \
        break; \
        case H_VAL_TYPE_INT16: \
        printf("%hd", *((int16_t *)r->val)); \
        break; \
        case H_VAL_TYPE_INT32: \
        printf("%d", *((int32_t *)r->val)); \
        break; \
        case H_VAL_TYPE_INT64: \
        printf("%lld", *((int64_t *)r->val)); \
        break; \
        case H_VAL_TYPE_STRING: \
        printf("%s", (char *)r->val); \
        break; \
        case H_VAL_TYPE_CHAR: \
        printf("%c", *((char *)r->val)); \
        break; \
        case H_VAL_TYPE_DOUBLE: \
        printf("%lf", *((double *)r->val)); \
        break; \
        case H_VAL_TYPE_FLOAT: \
        printf("%f", *((float *)r->val)); \
        break; \
    } \
} while(0) \

typedef struct hemers_hash_table_val_t {
        uint16_t                      val_type; /* val type */
        void                          *val;    /* point to value */
        uint32_t                      val_size;
} hemers_hash_table_val_t;

typedef struct hermes_hash_table_bucket_t {
        struct hermes_hash_table_bucket_t            *next;     /** bucket single-line table */
        struct hermes_hash_table_bucket_t            *index_next; /* next element by index size */
        const char                                   *key;      /** single bucket key */
        hemers_hash_table_val_t                      *value;    /** single bucket value */
        uint32_t                                     key_hash;  /** all buckets have the same key hash value*/
} hermes_hash_table_bucket_t;

typedef struct hermes_hash_table_t {
        uint32_t                                     ht_size; /** the bucket size of hash-table */
        uint32_t                                     ht_mask; /** the bucket size of hash-table */
        struct hermes_hash_table_bucket_t            **ht_buckets; /* bucket: data storage*/
        uint16_t                                     element_num; /* total elements num */
        hermes_hash_table_bucket_t                   *index_last;  /* last element of hastable */
        hermes_hash_table_bucket_t                   *index_first; /* first element of hashtable */
} hermes_hash_table_t;

typedef enum h_val_type_t {
    H_VAL_TYPE_UINT16   = 0x0001,
    H_VAL_TYPE_UINT32   = 0x0002,
    H_VAL_TYPE_UINT64   = 0x0004,
    H_VAL_TYPE_INT16    = 0x0008,
    H_VAL_TYPE_INT32    = 0x0010,
    H_VAL_TYPE_INT64    = 0x0020,
    H_VAL_TYPE_STRING   = 0x0040,
    H_VAL_TYPE_FLOAT    = 0x0080,
    H_VAL_TYPE_DOUBLE   = 0x0100,
    H_VAL_TYPE_CHAR     = 0x0200
} h_val_type_t;

hermes_hash_table_t * hermes_hash_table_init(int size);
int hermes_hash_table_insert(hermes_hash_table_t *hashtable, const char *key, void *value, uint16_t type, uint32_t size);
int hermes_hash_table_key_hash(const char *key, int *key_hash);
hemers_hash_table_val_t * hermes_hash_table_get_value(hermes_hash_table_t *hashtable, const char *key);
int hermes_hash_table_delete(hermes_hash_table_t *, const char *);
bool hermes_hash_table_check_init(hermes_hash_table_t *);
int hermes_hash_table_hgetall(hermes_hash_table_t *hashtable);
int hermes_hash_table_check_resize(hermes_hash_table_t *hashtable);
int hermes_hash_table_resize(hermes_hash_table_t *hashtable);
void hermes_hash_table_get_all_by_index_sort(hermes_hash_table_t *hashtable);

#endif