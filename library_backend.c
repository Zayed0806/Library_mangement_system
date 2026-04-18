#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define EXP __declspec(dllexport)
#define FILE_NAME "library.csv"
#define DELETED_FILE "deleted.csv"
#define MAXB 5000

/* ===== LOGGING FUNCTION DECLARATIONS ===== */
void log_add_book(int bookID, const char *title, const char *author,
                  const char *genre, int year, int copies);
void log_update_copies(int bookID, int oldCopies, int newCopies);
void log_delete_book(int bookID, const char *title);
void log_restore_book(int bookID, const char *title);
void log_sort(const char *mode);
void log_search(const char *mode, const char *query);
/* ======================================== */

typedef struct
{
    int bookID;
    char title[100];
    char author[100];
    char genre[100];
    int year;
    int copies;
} Book;

Book library[MAXB];
int n = 0;
Book deletedBooks[MAXB];
int deletedCount = 0;
int nextBookID = 1;

void rewriteDeletedCSV(void);
void mergeCopies(Book arr[], int left, int mid, int right);
void mergeSortCopies(Book arr[], int left, int right);

EXP void loadFromCSV(void)
{
    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) return;

    char line[512];
    fgets(line, sizeof(line), fp);

    n = 0;
    int maxID = 0;

    while (fscanf(fp, "%d,%99[^,],%99[^,],%99[^,],%d,%d\n",
                  &library[n].bookID,
                  library[n].title,
                  library[n].author,
                  library[n].genre,
                  &library[n].year,
                  &library[n].copies) == 6)
    {
        if (library[n].bookID > maxID)
            maxID = library[n].bookID;

        if (++n >= MAXB)
            break;
    }

    nextBookID = maxID + 1;
    fclose(fp);
}

EXP void saveToCSV()
{
    FILE *fp = fopen(FILE_NAME, "w");
    if (!fp) return;

    fprintf(fp, "BookID,Title,Author,Genre,Year,Copies\n");
    for (int i = 0; i < n; i++)
        fprintf(fp, "%d,%s,%s,%s,%d,%d\n",
                library[i].bookID,
                library[i].title,
                library[i].author,
                library[i].genre,
                library[i].year,
                library[i].copies);

    fclose(fp);
}

EXP int getBookCount() { return n; }
EXP Book getBook(int index) { return library[index]; }

EXP int addBook(char *title, char *author, char *genre, int year, int copies)
{
    if (n >= MAXB) return 0;

    Book b;
    b.bookID = ((b.year/100)*10000)+nextBookID++;
    strcpy(b.title, title);
    strcpy(b.author, author);
    strcpy(b.genre, genre);
    b.year = year;
    b.copies = copies;

    library[n++] = b;
    saveToCSV();

    /* 🔥 LOG */
    log_add_book(b.bookID, title, author, genre, year, copies);

    return 1;
}

EXP int updateBook(int bookID, int newCopies)
{
    for (int i = 0; i < n; i++)
        if (library[i].bookID == bookID)
        {
            int oldCopies = library[i].copies;
            library[i].copies = newCopies;
            saveToCSV();

            /* 🔥 LOG */
            log_update_copies(bookID, oldCopies, newCopies);

            return 1;
        }
    return 0;
}

EXP void sortByID()
{
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (library[j].bookID > library[j + 1].bookID)
            {
                Book t = library[j];
                library[j] = library[j + 1];
                library[j + 1] = t;
            }

    log_sort("ID");
}

EXP void sortByTitle()
{
    for (int i = 1; i < n; i++)
    {
        Book key = library[i];
        int j = i - 1;

        while (j >= 0 && strcmp(library[j].title, key.title) > 0)
        {
            library[j + 1] = library[j];
            j--;
        }
        library[j + 1] = key;
    }

    log_sort("TITLE");
}

EXP void sortByYear()
{
    for (int i = 0; i < n - 1; i++)
    {
        int min = i;
        for (int j = i + 1; j < n; j++)
            if (library[j].year < library[min].year)
                min = j;

        if (min != i)
        {
            Book t = library[i];
            library[i] = library[min];
            library[min] = t;
        }
    }

    log_sort("YEAR");
}

void mergeCopies(Book arr[], int left, int mid, int right)
{
    Book temp[MAXB];
    int i = left, j = mid + 1, k = left;

    while (i <= mid && j <= right)
        temp[k++] = (arr[i].copies <= arr[j].copies) ? arr[i++] : arr[j++];

    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];

    for (int x = left; x <= right; x++)
        arr[x] = temp[x];
}

void mergeSortCopies(Book arr[], int left, int right)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;
        mergeSortCopies(arr, left, mid);
        mergeSortCopies(arr, mid + 1, right);
        mergeCopies(arr, left, mid, right);
    }
}

EXP void sortByCopies()
{
    if (n > 1)
        mergeSortCopies(library, 0, n - 1);

    log_sort("COPIES");
}

EXP int searchByID(int bookID)
{
    log_search("ID", "");

    for (int i = 0; i < n; i++)
        if (library[i].bookID == bookID)
            return i;

    return -1;
}

EXP int searchByTitle(char *query, int *results)
{
    sortByTitle();  // Ensure sorted

    log_search("TITLE", query);

    int left = 0;
    int right = n - 1;
    int mid;
    int foundIndex = -1;
    size_t len = strlen(query);

    while (left <= right)
    {
        mid = (left + right) / 2;

        int cmp = strncmp(library[mid].title, query, len);

        if (cmp == 0)
        {
            foundIndex = mid;
            break;
        }
        else if (cmp < 0)
            left = mid + 1;
        else
            right = mid - 1;
    }

    if (foundIndex == -1)
        return 0;

    int count = 0;

    // scan left
    int i = foundIndex;
    while (i >= 0 &&
           strncmp(library[i].title, query, len) == 0)
        i--;

    i++;

    // scan right
    while (i < n &&
           strncmp(library[i].title, query, len) == 0)
        results[count++] = i++;

    return count;
}



void appendDeletedBookToCSV(Book b)
{
    FILE *fp = fopen(DELETED_FILE, "a");
    if (!fp) return;

    fprintf(fp, "%d,%s,%s,%s,%d,%d\n",
            b.bookID, b.title, b.author,
            b.genre, b.year, b.copies);

    fclose(fp);
}

EXP int deleteBook(int bookID)
{
    for (int i = 0; i < n; i++)
        if (library[i].bookID == bookID)
        {
            appendDeletedBookToCSV(library[i]);

            /* 🔥 LOG */
            log_delete_book(library[i].bookID, library[i].title);

            for (int j = i; j < n - 1; j++)
                library[j] = library[j + 1];

            n--;
            saveToCSV();
            return 1;
        }
    return 0;
}

EXP void loadDeletedFromCSV()
{
    FILE *fp = fopen(DELETED_FILE, "r");
    if (!fp) return;

    deletedCount = 0;
    char line[512];

    while (fgets(line, sizeof(line), fp))
    {
        Book b;
        if (sscanf(line, "%d,%99[^,],%99[^,],%99[^,],%d,%d",
                   &b.bookID, b.title, b.author,
                   b.genre, &b.year, &b.copies) == 6)
            deletedBooks[deletedCount++] = b;
    }
    fclose(fp);
}

EXP int getDeletedCount() { return deletedCount; }
EXP Book getDeletedBook(int index) { return deletedBooks[index]; }

EXP int restoreDeletedBook(int bookID)
{
    if (n >= MAXB) return 0;

    for (int i = 0; i < deletedCount; i++)
        if (deletedBooks[i].bookID == bookID)
        {
            library[n++] = deletedBooks[i];
            saveToCSV();

            /* 🔥 LOG */
            log_restore_book(bookID, deletedBooks[i].title);

            for (int j = i; j < deletedCount - 1; j++)
                deletedBooks[j] = deletedBooks[j + 1];

            deletedCount--;
            rewriteDeletedCSV();
            return 1;
        }
    return 0;
}

void rewriteDeletedCSV(void)
{
    FILE *fp = fopen(DELETED_FILE, "w");
    if (!fp) return;

    for (int i = 0; i < deletedCount; i++)
        fprintf(fp, "%d,%s,%s,%s,%d,%d\n",
                deletedBooks[i].bookID,
                deletedBooks[i].title,
                deletedBooks[i].author,
                deletedBooks[i].genre,
                deletedBooks[i].year,
                deletedBooks[i].copies);

    fclose(fp);
}

EXP int getLastGeneratedBookID(void)
{
    return nextBookID - 1;
}
