#include <stdio.h>
#include <string.h>
#include <time.h>

#define EXP __declspec(dllexport)
#define LOG_FILE "library_actions.csv"

static void get_timestamp(char *buf, size_t size)
{
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_s(&tm_now, &now);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm_now);
}

static void ensure_header(void)
{
    FILE *fp = fopen(LOG_FILE, "r");
    if (fp) {
        fclose(fp);
        return; // file already exists
    }

    fp = fopen(LOG_FILE, "w");
    if (!fp) return;

    fprintf(fp, "timestamp,action,details\n");
    fclose(fp);
}

static void write_line(const char *action, const char *details)
{
    if (!action || action[0] == '\0') return;

    ensure_header();

    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;

    char ts[32];
    get_timestamp(ts, sizeof(ts));

    if (details && details[0] != '\0')
        fprintf(fp, "\"%s\",\"%s\",\"%s\"\n", ts, action, details);
    else
        fprintf(fp, "\"%s\",\"%s\",\"\"\n", ts, action);

    fclose(fp);
}

EXP void log_init(void)
{
    ensure_header();
}

EXP void log_action(const char *action, const char *details)
{
    write_line(action, details);
}

EXP void log_add_book(int bookID, const char *title, const char *author,
                      const char *genre, int year, int copies)
{
    char details[512];
    snprintf(details, sizeof(details),
        "book_id=%d title=%s author=%s genre=%s year=%d copies=%d",
        bookID,
        title ? title : "",
        author ? author : "",
        genre ? genre : "",
        year,
        copies
    );
    write_line("ADD_BOOK", details);
}

EXP void log_update_copies(int bookID, int oldCopies, int newCopies)
{
    char details[256];
    snprintf(details, sizeof(details),
        "book_id=%d old_copies=%d new_copies=%d",
        bookID, oldCopies, newCopies
    );
    write_line("UPDATE_COPIES", details);
}

EXP void log_delete_book(int bookID, const char *title)
{
    char details[256];
    snprintf(details, sizeof(details),
        "book_id=%d title=%s",
        bookID, title ? title : ""
    );
    write_line("DELETE_BOOK", details);
}

EXP void log_restore_book(int bookID, const char *title)
{
    char details[256];
    snprintf(details, sizeof(details),
        "book_id=%d title=%s",
        bookID, title ? title : ""
    );
    write_line("RESTORE_BOOK", details);
}

EXP void log_sort(const char *mode)
{
    char details[128];
    snprintf(details, sizeof(details),
        "mode=%s",
        mode ? mode : ""
    );
    write_line("SORT", details);
}

EXP void log_search(const char *mode, const char *query)
{
    char details[256];
    snprintf(details, sizeof(details),
        "mode=%s query=%s",
        mode ? mode : "",
        query ? query : ""
    );
    write_line("SEARCH", details);
}
