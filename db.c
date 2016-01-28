#include <string.h>
#include "db.h"

static int db_enabled = 0;

static sqlite3 *db;
static sqlite3_stmt *insert_song_stmt;
static sqlite3_stmt *get_song_path_stmt;
static sqlite3_stmt *update_song_state_stmt;
static sqlite3_stmt *delete_song_stmt;
static const char *reset_song_state_query = "update songs set is_playing = 0";

void db_enable() { db_enabled = 1; }

void db_disable() { db_enabled = 0; }

int get_db_enabled() { return db_enabled; }

int db_init(char *path) {
    if (!db_enabled) {
        return 0;
    }
    static const char *create_query = "create table if not exists songs ("
                                      "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                      "   name VARCHAR(255) NOT NULL,"
                                      "   path VARCHAR(512),"
                                      "   is_playing INTEGER(2) DEFAULT 0"
                                      ");";
    static const char *insert_song_query = "insert into songs (name, path) "
                                           "values (?, ?);";
    static const char *get_song_path_query =
        "select path from songs where id = ?;";
    static const char *update_song_state_query =
        "update songs set is_playing = ? where id = ?;";
    static const char *delete_song_query = "delete from songs where id = ?;";
    int rc;
    rc = sqlite3_open(path, &db);
    if (rc)
        return rc;
    rc = sqlite3_exec(db, create_query, NULL, NULL, NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, insert_song_query, -1, &insert_song_stmt, NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, get_song_path_query, -1, &get_song_path_stmt,
                            NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, update_song_state_query, -1,
                            &update_song_state_stmt, NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, delete_song_query, -1, &delete_song_stmt, NULL);
    if (rc)
        return rc;
    /* sqlite3_exec(db, "begin;", NULL, NULL, NULL); */
    return 0;
}

void db_close() {
    if (!db_enabled) {
        return;
    }
    /* sqlite3_exec(db, "commit;", NULL, NULL, NULL); */
    sqlite3_finalize(insert_song_stmt);
    sqlite3_finalize(update_song_state_stmt);
    sqlite3_finalize(delete_song_stmt);
    sqlite3_close(db);
}

void db_insert_song(char *name, char *path) {
    if (!db_enabled) {
        return;
    }
    sqlite3_reset(insert_song_stmt);
    sqlite3_bind_text(insert_song_stmt, 1, name, -1, NULL);
    sqlite3_bind_text(insert_song_stmt, 2, path, -1, NULL);
    sqlite3_step(insert_song_stmt);
}

void db_update_song_state(int is_playing, int id) {
    if (!db_enabled) {
        return;
    }
    sqlite3_exec(db, reset_song_state_query, NULL, NULL, NULL);
    sqlite3_reset(update_song_state_stmt);
    sqlite3_bind_int(update_song_state_stmt, 1, is_playing);
    sqlite3_bind_int(update_song_state_stmt, 2, id);
    sqlite3_step(update_song_state_stmt);
}

void db_delete_song(int id) {
    if (!db_enabled) {
        return;
    }
    sqlite3_reset(delete_song_stmt);
    sqlite3_bind_int(delete_song_stmt, 1, id);
    sqlite3_step(delete_song_stmt);
}

char *get_song_path(int id) {
    char *path = 0;
    if (!db_enabled) {
        return path;
    }
    sqlite3_reset(get_song_path_stmt);
    sqlite3_bind_int(get_song_path_stmt, 1, id);
    if (sqlite3_step(get_song_path_stmt) == SQLITE_ROW) {
        path = (char *)sqlite3_column_text(get_song_path_stmt, 0);
    }
    return path;
}

int db_load_songs(char ***result, int *nrow, int *ncolumn, char **pzErrmsg) {
    return sqlite3_get_table(db, "select id, name from songs", result, nrow, ncolumn,
                             pzErrmsg);
}
