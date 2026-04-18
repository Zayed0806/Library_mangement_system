#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define EXP __declspec(dllexport)

static FILE *fp = NULL;

EXP void init(void)
{
    srand((unsigned)time(NULL));
}

static const char first_names[][100] = {
    "William","George","Jane","Emily","Charles","Mark",
    "Ernest","Virginia","Leo","Fyodor","Franz","Oscar",
    "Gabriel","Haruki","Jorge","Paulo","Kurt","Arthur",
    "Toni","Margaret","Stephen","Agatha","Herman","James",
    "John","Ray","Neil","Isaac","Ursula","Aldous",
    "Amit","Anand","Arjun","Ashok","Deepak","Kiran",
    "Manoj","Rahul","Ravi","Sanjay","Sunil","Vikas",
    "Vivek","Prakash","Rajesh","Suresh","Mahesh",
    "Nikhil","Rohan","Aditya","Aarav","Ishaan",
    "Neha","Anita","Pooja","Kavita","Meera",
    "Shreya","Ananya","Priya"
};

static const char last_names[][100] = {
    "Shakespeare","Orwell","Austen","Bronte","Dickens",
    "Twain","Hemingway","Woolf","Tolstoy","Dostoevsky",
    "Kafka","Wilde","Marquez","Murakami","Borges",
    "Coelho","Vonnegut","ConanDoyle","Morrison","Atwood",
    "King","Christie","Melville","Joyce","Steinbeck",
    "Bradbury","Gaiman","Asimov","LeGuin","Huxley",
    "Sharma","Verma","Gupta","Mehta","Patel",
    "Singh","Kumar","Malhotra","Chatterjee","Banerjee",
    "Mukherjee","Iyer","Iyengar","Rao","Reddy",
    "Naidu","Pillai","Nair","Menon","Das",
    "Ghosh","Bose","Kapoor","Khanna","Bhat",
    "Shetty","Joshi","Kulkarni","Deshmukh","Pawar"
};

static const char genres[][50] = {
    "Fiction","Non-Fiction","Science Fiction","Fantasy",
    "Mystery","Thriller","Romance","Historical",
    "Biography","Autobiography","Poetry","Drama",
    "Horror","Adventure","Young","Children",
    "Philosophy","Psychology","Self Help","Business",
    "Economics","Politics","Sociology","Education",
    "Technology","Programming","Mathematics","Physics",
    "Chemistry","Biology"
};

static const char adjectives[][50] = {
    "Silent","Broken","Hidden","Lost","Dark","Golden",
    "Final","Forgotten","Ancient","Infinite","Crimson",
    "Lonely","Fading","Shattered","Eternal","Restless",
    "Burning","Frozen","Wandering","Secret","Midnight",
    "Sacred","Cursed","Hollow","Distant","Rising",
    "Fallen","Unseen","Twisted","Timeless"
};

static const char nouns[][50] = {
    "Path","Mirror","Empire","Horizon","Dreams","Shadows",
    "Voices","Chronicle","Kingdom","Time","Memory","Fire",
    "Ashes","Stars","Storm","Light","Darkness","Truth",
    "Fate","Silence","Legacy","Worlds","Night","Echoes",
    "Secrets","Blood","Power","Hope","Journey","Myth"
};

static const char connectors[][30] = {
    "of","in","and","from","to","between",
    "beyond","within","under","over"
};

EXP int open_csv(const char *filename)
{
    fp = fopen(filename, "w");
    if (!fp) return 0;
    fprintf(fp, "BookID,Title,Author,Genre,Year,Copies\n");
    return 1;
}

EXP void list_gen(int n)
{
    if (!fp) return;

    char title[150];
    char author[150];

    for (int i = 0; i < n; i++)
    {
        int year = 1901 + rand() % (2026 - 1900 + 1);
        int copies = 1 + rand() % 100; 
        int bookID = ((year/100) * 100000) + (i + 1);   

        int a = rand() % 61;
        int b = rand() % 60;
        int g = rand() % 30;
        int adj = rand() % 30;
        int noun = rand() % 30;
        int con = rand() % 10;

        sprintf(author, "%s %s", first_names[a], last_names[b]);
        sprintf(title, "%s %s %s", adjectives[adj], connectors[con], nouns[noun]);

        fprintf(fp, "%d,%s,%s,%s,%d,%d\n",
                bookID, title, author, genres[g], year, copies);
    }
}

EXP void close_csv(void)
{
    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }
}
