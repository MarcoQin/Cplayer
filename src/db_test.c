#include <stdio.h>
#include "db.h"

int main(int argc, char *argv[]) {
    db_enable();
    int rc = db_init("./songs.db");
    if (rc != 0) {
        fprintf(stderr, "Fail to init db.\n");
    }
    printf("Initialize database\n");
    /* int i; */
    /* char name[255]; */
    /* char path[255]; */
    /* for (i = 0; i < 10; i++) */
    /* { */
    /* sprintf(name, "Song#%i", i); */
    /* sprintf(path, "Path#%i", i); */
    /* db_insert_song(name, path); */
    /* } */
    char **result = 0;
    int i, j, nrow, ncol, index;
    char *errmsg;
    rc = db_load_songs(&result, &nrow, &ncol, &errmsg);
    if (rc == 0) {
        printf("query %i records.\n", nrow);
        index = ncol;
        for (i = 0; i < nrow; i++) {
            printf("[%2i]", i);
            for (j = 0; j < ncol; j++) {
                printf(" %s", result[index]);
                index++;
            }
            printf("\n");
        }
    }

    sqlite3_free_table(result);
    sqlite3_free(errmsg);
    db_close();
    db_disable();
    return 0;
}
