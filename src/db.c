#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sqlite3/sqlite3.h"
#include "db.h"
#include "utils.h"

static int db_enabled = 0;

static sqlite3 *db;
static sqlite3_stmt *insert_song_stmt;
static sqlite3_stmt *get_song_path_stmt;
static sqlite3_stmt *get_song_name_stmt;
static sqlite3_stmt *get_current_playing_song_stmt;
static sqlite3_stmt *get_next_song_id_stmt;
static sqlite3_stmt *get_previous_song_id_stmt;
static sqlite3_stmt *update_song_state_stmt;
static sqlite3_stmt *delete_song_stmt;
static sqlite3_stmt *get_first_song_id_stmt;
static sqlite3_stmt *get_last_song_id_stmt;
static const char *reset_song_state_query = "update songs set is_playing = 0;";

void db_enable() { db_enabled = 1; }

void db_disable() { db_enabled = 0; }

int get_db_enabled() { return db_enabled; }

int db_init(const char *path) {
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
    static const char *get_song_name_query =
        "select name from songs where id = ?;";
    static const char *get_current_playing_song_query =
        "select id from songs where is_playing = 1;";
    static const char *get_next_song_id_query =
        "select id from songs where id > ? order by id limit 1;";
    static const char *get_previous_song_id_query =
        "select id from songs where id < ? order by id desc limit 1;";
    static const char *update_song_state_query =
        "update songs set is_playing = ? where id = ?;";
    static const char *delete_song_query = "delete from songs where id = ?;";
    static const char *get_first_song_id_query =
                    "select id from songs limit 1;";
    static const char *get_last_song_id_query =
                    "select id from songs order by id desc limit 1;";
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
    rc = sqlite3_prepare_v2(db, get_song_name_query, -1, &get_song_name_stmt,
                            NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, get_current_playing_song_query, -1, &get_current_playing_song_stmt,
                            NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, get_next_song_id_query, -1, &get_next_song_id_stmt,
                            NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, get_previous_song_id_query, -1, &get_previous_song_id_stmt,
                            NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, get_first_song_id_query, -1,
                            &get_first_song_id_stmt, NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, get_last_song_id_query, -1,
                            &get_last_song_id_stmt, NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, update_song_state_query, -1,
                            &update_song_state_stmt, NULL);
    if (rc)
        return rc;
    rc = sqlite3_prepare_v2(db, delete_song_query, -1, &delete_song_stmt, NULL);
    if (rc)
        return rc;
    return 0;
}

void db_close() {
    if (!db_enabled) {
        return;
    }
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

char *get_song_name(int id) {
    char *name = 0;
    if (!db_enabled) {
        return name;
    }
    sqlite3_reset(get_song_name_stmt);
    sqlite3_bind_int(get_song_name_stmt, 1, id);
    if (sqlite3_step(get_song_name_stmt) == SQLITE_ROW) {
        name = (char *)sqlite3_column_text(get_song_name_stmt, 0);
    }
    return name;
}

int get_next_or_previous_song_id(int type) {
    int id = 0;
    if (!db_enabled) {
        return id;
    }
    sqlite3_reset(get_current_playing_song_stmt);
    if (sqlite3_step(get_current_playing_song_stmt) == SQLITE_ROW) {
        id = sqlite3_column_int(get_current_playing_song_stmt, 0);
    }
    if (type == NEXT)
    {
        sqlite3_reset(get_next_song_id_stmt);
        sqlite3_bind_int(get_next_song_id_stmt, 1, id);
        if (sqlite3_step(get_next_song_id_stmt) == SQLITE_ROW) {
            id = sqlite3_column_int(get_next_song_id_stmt, 0);
        } else {
            if (sqlite3_step(get_first_song_id_stmt) == SQLITE_ROW) {
                id = sqlite3_column_int(get_first_song_id_stmt, 0);
            }
        }
    } else {
        sqlite3_reset(get_previous_song_id_stmt);
        sqlite3_bind_int(get_previous_song_id_stmt, 1, id);
        if (sqlite3_step(get_previous_song_id_stmt) == SQLITE_ROW) {
            id = sqlite3_column_int(get_previous_song_id_stmt, 0);
        } else {
            if (sqlite3_step(get_last_song_id_stmt) == SQLITE_ROW) {
                id = sqlite3_column_int(get_last_song_id_stmt, 0);
            }
        }
    }
    return id;
}

int db_load_songs(char ***result, int *nrow, int *ncolumn, char **pzErrmsg) {
    return sqlite3_get_table(db, "select id, name from songs", result, nrow, ncolumn,
                             pzErrmsg);
}

int loading_choices(char **choices)
{
    char **result = 0;
    int i, j, nrow, ncol, index, rc;
    char *errmsg;
    rc = db_load_songs(&result, &nrow, &ncol, &errmsg);
    if (rc == 0) {
        index = ncol;
        for (i = 0; i < nrow; i++) {
            for (j = 0; j < ncol; j++) {
                if (j == 1)
                {
                    char *dot = ".";
                    char *r = merge_str(result[index-1], dot, result[index]);
                    choices[i] = r;
                }
                index++;
            }
        }
    }
    return nrow;
}
