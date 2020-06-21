#include "map.h"

hermes_hash_table_t * hermes_hash_table_init(int size)
{
    hermes_hash_table_t *hashtable = (hermes_hash_table_t *) malloc(sizeof(hermes_hash_table_t));
    memset(hashtable, 0, sizeof(hermes_hash_table_t));

    if (size > MAX_HASH_TABLE_SIZE) {
        printf("over maximun hashtable size\n");
        return NULL;
    }

    uint16_t i = 0;
    while((1U << i) < size) {
        i++;
    }
    size = 1 << i;

    hashtable->ht_size = size;
    hashtable->ht_mask = size - 1;
    hashtable->element_num = 0;
    hashtable->index_first =  NULL;
    hashtable->index_last  = NULL;

    hashtable->ht_buckets = (hermes_hash_table_bucket_t **) malloc(hashtable->ht_size * sizeof(hermes_hash_table_bucket_t *));
    memset(hashtable->ht_buckets, 0, hashtable->ht_size * sizeof(hermes_hash_table_bucket_t *));

    return hashtable;
}

int hermes_hash_table_insert(hermes_hash_table_t *hashtable, const char *key, void *value, uint16_t type, uint32_t size)
{
    int key_hash, bukcet_index;
    hermes_hash_table_bucket_t **_bucket,*insert_bucket,*index_bucket;

    if (hermes_hash_table_check_resize(hashtable)) {
        hermes_hash_table_resize(hashtable);
    }

    hermes_hash_table_key_hash(key, &key_hash);
    bukcet_index = HT_GET_INDEX(key_hash, hashtable);
    _bucket = &(hashtable->ht_buckets[bukcet_index]);

    if ((*_bucket) == NULL) {
        *_bucket = (hermes_hash_table_bucket_t *) malloc(sizeof(hermes_hash_table_bucket_t));
        memset(*_bucket, 0, sizeof(hermes_hash_table_bucket_t));
        HT_BUCKET_INIT((*_bucket), key, key_hash, value, size, type);
        insert_bucket = *_bucket;
    } else {
        hermes_hash_table_bucket_t *tmp_bucket = *_bucket;
        while(tmp_bucket != NULL) {
            if (strcmp((const char *)tmp_bucket->key, key) == 0) {
                HERMES_VAL_COPY(tmp_bucket, value, size, type);
                return 0;
            }

            if (tmp_bucket->next == NULL) {
                break;
            } else {
                tmp_bucket = tmp_bucket->next;
            }
        }
        
        tmp_bucket->next = (hermes_hash_table_bucket_t *) malloc(sizeof(hermes_hash_table_bucket_t));
        memset(tmp_bucket->next, 0, sizeof(hermes_hash_table_bucket_t));
        HT_BUCKET_INIT(tmp_bucket->next, key, key_hash, value, size, type);
        insert_bucket = tmp_bucket->next;
    }

    /* first insert */
    if (hashtable->element_num == 0) {
        hashtable->index_first = insert_bucket;
        hashtable->index_last  = insert_bucket;
        insert_bucket->index_next = NULL;
    } else {
        index_bucket = hashtable->index_first;
        if (insert_bucket->key_hash < index_bucket->key_hash) {
            insert_bucket->index_next = index_bucket;
            hashtable->index_first = insert_bucket;
        } else {
            do {
                if (index_bucket->index_next == NULL || insert_bucket->key_hash <= index_bucket->index_next->key_hash) {
                    insert_bucket->index_next = index_bucket->index_next;
                    index_bucket->index_next = insert_bucket;
                    break;
                }
                index_bucket = index_bucket->index_next;
            } while (index_bucket != NULL);
        }
    }

    hashtable->element_num++;
    return 0;
}

int hermes_hash_table_key_hash(const char *key, int *key_hash)
{
    *key_hash = 0;
    while(*key != '\0') {
        *key_hash += *key;
        key++;
    }

    return 0;
}

hemers_hash_table_val_t * hermes_hash_table_get_value(hermes_hash_table_t *hashtable, const char *key)
{
    int key_hash;
    hermes_hash_table_key_hash(key, &key_hash);

    hermes_hash_table_bucket_t *tmp_bucket = hashtable->ht_buckets[HT_GET_INDEX(key_hash, hashtable)];

    while(tmp_bucket != NULL) {
        if (strcmp(tmp_bucket->key, key) == 0) {
            return tmp_bucket->value;
        }
        tmp_bucket = tmp_bucket->next;
    }

    return NULL;
}

int hermes_hash_table_delete(hermes_hash_table_t * hashtable, const char *key) 
{
    int key_hash;
    hermes_hash_table_key_hash(key, &key_hash);

    hermes_hash_table_bucket_t *tmp_bucket = hashtable->ht_buckets[HT_GET_INDEX(key_hash, hashtable)];

    while(tmp_bucket != NULL) {
        if (strcmp((const char *)tmp_bucket->key, key) == 0) {
            if (tmp_bucket->next == NULL) {
                free(tmp_bucket);
                hashtable->ht_buckets[HT_GET_INDEX(key_hash, hashtable)] = NULL;
                return 0;
            } else {
                hermes_hash_table_bucket_t * tmp_next = tmp_bucket->next;
                tmp_bucket->next = tmp_next->next;
                tmp_next->next = NULL;
            }
        }
        tmp_bucket = tmp_bucket->next;
    }

    return 0;
}

bool hermes_hash_table_check_init(hermes_hash_table_t *hashtable) 
{
    return hashtable->ht_size > 0;
}

int hermes_hash_table_hgetall(hermes_hash_table_t *hashtable)
{
    if (!hermes_hash_table_check_init(hashtable)) {
        printf("hashtable not initialized\n");
        return 1;
    }

    if (hashtable->element_num == 0) {
        printf("hashtable empty\n");
        return 1;
    }

    for(int i = 0;i < hashtable->ht_size;i++) {
        hermes_hash_table_bucket_t *_bucket = hashtable->ht_buckets[i];
        printf("bucket %d:\n", i);
        while(_bucket != NULL) {
            printf("%s: ", _bucket->key);
            HVAL_PRINT(_bucket->value);
            printf("\n");
            _bucket = _bucket->next;
        }
        printf("\n");
    }

    return 0;
}

void hermes_hash_table_get_all_by_index_sort(hermes_hash_table_t *hashtable)
{
    if (hashtable->element_num == 0) {
        printf("hashtable empty\n");
        exit(0);
    }

    hermes_hash_table_bucket_t *index_bucket;
    index_bucket = hashtable->index_first;
    
    do {
        printf("%s: ", index_bucket->key);
        HVAL_PRINT(index_bucket->value);
        printf("(%d)\n", index_bucket->key_hash);
        index_bucket = index_bucket->index_next;
    } while (index_bucket != NULL);
}

int hermes_hash_table_check_resize(hermes_hash_table_t *hashtable)
{
    if (hashtable->element_num / hashtable->ht_size > HT_RESIZE_FACTORY) {
        return 1;
    }
    return 0;
}

int hermes_hash_table_resize(hermes_hash_table_t *hashtable)
{
    uint32_t size = hashtable->ht_size;
    hermes_hash_table_bucket_t **tmp_buckets, **ori_buckets, *tmp_bucket, *free_bucket;
    ori_buckets = hashtable->ht_buckets;

    hashtable->ht_size <<= 1;
    hashtable->ht_mask = hashtable->ht_size - 1;
    hashtable->element_num = 0;

    tmp_buckets = (hermes_hash_table_bucket_t **) malloc(sizeof(hermes_hash_table_bucket_t *) * hashtable->ht_size);
    memset(tmp_buckets, 0, sizeof(hermes_hash_table_bucket_t *));
    hashtable->ht_buckets = tmp_buckets;

    for(uint16_t i = 0;i < size;i++) {
        tmp_bucket = ori_buckets[i];
        while(tmp_bucket != NULL) {
            hermes_hash_table_insert(hashtable, tmp_bucket->key, tmp_bucket->value->val, tmp_bucket->value->val_type, tmp_bucket->value->val_size);
            free_bucket = tmp_bucket;
            tmp_bucket = tmp_bucket->next;
            free(free_bucket);
        }
    }

    free(ori_buckets);
    return 0;
}