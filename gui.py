import ctypes
import tkinter as tk
from tkinter import ttk, messagebox

# ================= CONSTANTS =================

LOG_FILE = "library_actions.csv"

# ================= ctypes STRUCT =================

class Book(ctypes.Structure):
    _fields_ = [
        ("bookID", ctypes.c_int),
        ("title", ctypes.c_char * 100),
        ("author", ctypes.c_char * 100),
        ("genre", ctypes.c_char * 100),
        ("year", ctypes.c_int),
        ("copies", ctypes.c_int),
    ]

# ================= BACKEND DLL =================

lib = ctypes.CDLL("./library.dll")

lib.loadFromCSV()
lib.loadDeletedFromCSV()

lib.getBookCount.restype = ctypes.c_int
lib.getBook.argtypes = [ctypes.c_int]
lib.getBook.restype = Book

lib.getDeletedCount.restype = ctypes.c_int
lib.getDeletedBook.argtypes = [ctypes.c_int]
lib.getDeletedBook.restype = Book

lib.addBook.argtypes = [
    ctypes.c_char_p, ctypes.c_char_p,
    ctypes.c_char_p, ctypes.c_int, ctypes.c_int
]
lib.addBook.restype = ctypes.c_int

lib.updateBook.argtypes = [ctypes.c_int, ctypes.c_int]
lib.updateBook.restype = ctypes.c_int

lib.deleteBook.argtypes = [ctypes.c_int]
lib.deleteBook.restype = ctypes.c_int

lib.restoreDeletedBook.argtypes = [ctypes.c_int]
lib.restoreDeletedBook.restype = ctypes.c_int

lib.searchByID.argtypes = [ctypes.c_int]
lib.searchByID.restype = ctypes.c_int

lib.searchByTitle.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_int)]
lib.searchByTitle.restype = ctypes.c_int

lib.sortByID.argtypes = []
lib.sortByTitle.argtypes = []
lib.sortByYear.argtypes = []
lib.sortByCopies.argtypes = []

# ================= GENERATOR DLL =================

gen = ctypes.CDLL("./data_gen.dll")

gen.init.argtypes = []
gen.open_csv.argtypes = [ctypes.c_char_p]
gen.open_csv.restype = ctypes.c_int
gen.list_gen.argtypes = [ctypes.c_int]
gen.close_csv.argtypes = []

# ================= GUI =================

root = tk.Tk()
root.title("Library Management System")
root.geometry("1200x720")

main_frame = tk.Frame(root)
main_frame.pack(fill="both", expand=True)

# ================= TREEVIEW =================

tree_frame = tk.Frame(main_frame)
tree_frame.pack(side="left", fill="both", expand=True, padx=10, pady=10)

columns = ("ID", "Title", "Author", "Genre", "Year", "Copies")
tree = ttk.Treeview(tree_frame, columns=columns, show="headings")

for col in columns:
    tree.heading(col, text=col)
    tree.column(col, anchor="center", width=130)

tree.pack(fill="both", expand=True)

control_frame = tk.Frame(main_frame, width=350)
control_frame.pack(side="right", fill="y", padx=10, pady=10)

# ================= CORE FUNCTIONS =================

def refresh_table():
    tree.delete(*tree.get_children())
    for i in range(lib.getBookCount()):
        b = lib.getBook(i)
        tree.insert("", "end", values=(
            b.bookID,
            b.title.decode(),
            b.author.decode(),
            b.genre.decode(),
            b.year,
            b.copies
        ))

def clear_add_fields():
    title_e.delete(0, tk.END)
    author_e.delete(0, tk.END)
    genre_e.delete(0, tk.END)
    year_e.delete(0, tk.END)
    copies_e.delete(0, tk.END)

# ================= SORT =================

def sort_by_id():
    lib.sortByID()
    refresh_table()

def sort_by_title():
    lib.sortByTitle()
    refresh_table()

def sort_by_year():
    lib.sortByYear()
    refresh_table()

def sort_by_copies():
    lib.sortByCopies()
    refresh_table()

# ================= CRUD =================

def add_book():
    try:
        lib.addBook(
            title_e.get().encode(),
            author_e.get().encode(),
            genre_e.get().encode(),
            int(year_e.get()),
            int(copies_e.get())
        )
        refresh_table()
        clear_add_fields()
        messagebox.showinfo("Success", "Book added")
    except:
        messagebox.showerror("Error", "Invalid input")

def update_book():
    try:
        if not lib.updateBook(int(update_id_e.get()), int(update_copies_e.get())):
            messagebox.showinfo("Result", "Book not found")
        refresh_table()
    except:
        messagebox.showerror("Error", "Invalid input")

def delete_book():
    try:
        bid = int(delete_id_e.get())
        if messagebox.askyesno("Confirm", f"Delete book ID {bid}?"):
            lib.deleteBook(bid)
            lib.loadDeletedFromCSV()
            refresh_table()
    except:
        messagebox.showerror("Error", "Invalid ID")

# ================= SEARCH =================

def search_id(event=None):
    search_title_e.delete(0, tk.END)
    v = search_id_e.get()
    if not v:
        refresh_table()
        return
    idx = lib.searchByID(int(v))
    tree.delete(*tree.get_children())
    if idx >= 0:
        b = lib.getBook(idx)
        tree.insert("", "end", values=(
            b.bookID,
            b.title.decode(),
            b.author.decode(),
            b.genre.decode(),
            b.year,
            b.copies
        ))

def search_title(event=None):
    search_id_e.delete(0, tk.END)

    q = search_title_e.get()
    if not q:
        refresh_table()
        return

    lib.sortByTitle()   # 🔥 ensure sorted

    results = (ctypes.c_int * 1000)()
    count = lib.searchByTitle(q.encode(), results)

    tree.delete(*tree.get_children())
    for i in range(count):
        b = lib.getBook(results[i])
        tree.insert("", "end", values=(
            b.bookID,
            b.title.decode(),
            b.author.decode(),
            b.genre.decode(),
            b.year,
            b.copies
        ))


# ================= DELETED BOOKS =================

def open_deleted_books():
    lib.loadDeletedFromCSV()

    win = tk.Toplevel(root)
    win.title("Deleted Books")
    win.geometry("900x400")

    tree_d = ttk.Treeview(win, columns=columns, show="headings")

    for col in columns:
        tree_d.heading(col, text=col)
        tree_d.column(col, anchor="center", width=120)

    tree_d.pack(fill="both", expand=True, padx=10, pady=10)

    for i in range(lib.getDeletedCount()):
        b = lib.getDeletedBook(i)
        tree_d.insert("", "end", values=(
            b.bookID,
            b.title.decode(),
            b.author.decode(),
            b.genre.decode(),
            b.year,
            b.copies
        ))

    def restore_selected():
        sel = tree_d.selection()
        if not sel:
            return
        bid = tree_d.item(sel[0])["values"][0]
        if lib.restoreDeletedBook(bid):
            lib.loadDeletedFromCSV()
            refresh_table()
            win.destroy()
            messagebox.showinfo("Restored", f"Book ID {bid} restored")

    tk.Button(win, text="Restore Selected Book", bg="green", fg="white",
              command=restore_selected).pack(pady=10)

# ================= LOG VIEWER =================
def open_log_viewer():
    win = tk.Toplevel(root)
    win.title("Library Action Logs")
    win.geometry("900x500")

    # ================= Container =================
    container = tk.Frame(win)
    container.pack(fill="both", expand=True)

    # ================= Treeview =================
    tree = ttk.Treeview(container, show="headings")

    yscroll = ttk.Scrollbar(container, orient="vertical", command=tree.yview)
    xscroll = ttk.Scrollbar(container, orient="horizontal", command=tree.xview)

    tree.configure(yscrollcommand=yscroll.set, xscrollcommand=xscroll.set)

    tree.pack(side="left", fill="both", expand=True)
    # yscroll.pack(side="right", fill="y")
    # xscroll.pack(side="bottom", fill="x")

    # ================= Functions =================
    def load_logs():
        tree.delete(*tree.get_children())

        try:
            with open(LOG_FILE, newline="", encoding="utf-8") as f:
                reader = csv.reader(f)
                rows = list(reader)

                if not rows:
                    return

                headers = rows[0]
                data = rows[1:]

                tree["columns"] = headers

                for col in headers:
                    tree.heading(col, text=col)
                    tree.column(col, anchor="center", width=150)

                for row in data:
                    tree.insert("", "end", values=row)

        except FileNotFoundError:
            tree["columns"] = ("Message",)
            tree.heading("Message", text="Message")
            tree.insert("", "end", values=("Log file not found.",))

    def clear_logs():
        if messagebox.askyesno("Confirm", "Clear all logs?"):
            with open(LOG_FILE, "w", newline="", encoding="utf-8") as f:
                pass
            load_logs()

    # ================= Buttons =================
    btns = tk.Frame(win)
    btns.pack(fill="x", pady=5)
    tk.Button(btns, text="Clear Logs", command=clear_logs).pack(side="left", padx=5)
    load_logs()

# ================= CSV GENERATOR =================

def generate_csv():
    win = tk.Toplevel(root)
    win.title("Generate CSV")
    win.geometry("300x150")

    tk.Label(win, text="Number of books").pack(pady=5)
    count_e = tk.Entry(win)
    count_e.pack()

    def generate():
        try:
            n = int(count_e.get())
            gen.init()
            gen.open_csv(b"library.csv")
            gen.list_gen(n)
            gen.close_csv()
            lib.loadFromCSV()
            refresh_table()
            messagebox.showinfo("Success", f"{n} books generated")
            win.destroy()
        except:
            messagebox.showerror("Error", "Invalid number")

    tk.Button(win, text="Generate", bg="blue", fg="white",
              command=generate).pack(pady=10)

# ================= UI PANELS =================

# ADD
add = tk.LabelFrame(control_frame, text="Add Book")
add.pack(fill="x", pady=5)

labels = ["Title", "Author", "Genre", "Year", "Copies"]
entries = []

for i, l in enumerate(labels):
    tk.Label(add, text=l).grid(row=i, column=0)
    e = tk.Entry(add)
    e.grid(row=i, column=1)
    entries.append(e)

title_e, author_e, genre_e, year_e, copies_e = entries
tk.Button(add, text="Add Book", command=add_book).grid(row=6, columnspan=2)

# SEARCH
search = tk.LabelFrame(control_frame, text="Search")
search.pack(fill="x")

tk.Label(search, text="ID").grid(row=0, column=0)
search_id_e = tk.Entry(search)
search_id_e.grid(row=0, column=1)
search_id_e.bind("<KeyRelease>", search_id)

tk.Label(search, text="Title").grid(row=1, column=0)
search_title_e = tk.Entry(search)
search_title_e.grid(row=1, column=1)
search_title_e.bind("<KeyRelease>", search_title)

# SORT
sort = tk.LabelFrame(control_frame, text="Sort")
sort.pack(fill="x")

tk.Button(sort, text="ID (Bubble)", command=sort_by_id).pack(fill="x")
tk.Button(sort, text="Title (Insertion)", command=sort_by_title).pack(fill="x")
tk.Button(sort, text="Year (Selection)", command=sort_by_year).pack(fill="x")
tk.Button(sort, text="Copies (Merge)", command=sort_by_copies).pack(fill="x")

# UPDATE
update = tk.LabelFrame(control_frame, text="Update Copies")
update.pack(fill="x")

tk.Label(update, text="ID").grid(row=0, column=0)
update_id_e = tk.Entry(update)
update_id_e.grid(row=0, column=1)

tk.Label(update, text="Copies").grid(row=1, column=0)
update_copies_e = tk.Entry(update)
update_copies_e.grid(row=1, column=1)

tk.Button(update, text="Update", command=update_book).grid(row=2, columnspan=2)

# DELETE
delete = tk.LabelFrame(control_frame, text="Delete")
delete.pack(fill="x")

tk.Label(delete, text="ID").grid(row=0, column=0)
delete_id_e = tk.Entry(delete)
delete_id_e.grid(row=0, column=1)

tk.Button(delete, text="Delete", bg="red", fg="white",
          command=delete_book).grid(row=1, columnspan=2)

# VIEW DELETED / LOGS / GENERATE
tk.Button(control_frame, text="View Deleted Books",
          bg="#444", fg="white",
          command=open_deleted_books).pack(fill="x", pady=5)

tk.Button(control_frame, text="View Action Logs",
          bg="#222", fg="white",
          command=open_log_viewer).pack(fill="x", pady=5)

tk.Button(control_frame, text="Generate CSV (Auto)",
          bg="#0066cc", fg="white",
          command=generate_csv).pack(fill="x", pady=10)

# ================= START =================

refresh_table()
root.mainloop()
