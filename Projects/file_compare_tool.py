import tkinter as tk
from tkinter import filedialog, messagebox

def browse_file1():
    filepath = filedialog.askopenfilename(title="Select First File")
    if filepath:
        entry_file1.delete(0, tk.END)
        entry_file1.insert(0, filepath)

def browse_file2():
    filepath = filedialog.askopenfilename(title="Select Second File")
    if filepath:
        entry_file2.delete(0, tk.END)
        entry_file2.insert(0, filepath)

def compare_files():
    file1 = entry_file1.get()
    file2 = entry_file2.get()

    if not file1 or not file2:
        messagebox.showwarning("Warning", "Please select both files!")
        return

    try:
        with open(file1, "rb") as f1, open(file2, "rb") as f2:
            content1 = f1.read()
            content2 = f2.read()

            if content1 == content2:
                result_label.config(text="Files are IDENTICAL ✅", fg="green")
            else:
                result_label.config(text="Files are DIFFERENT ❌", fg="red")
    except Exception as e:
        messagebox.showerror("Error", f"Error comparing files:\n{str(e)}")

# GUI setup
root = tk.Tk()
root.title("File Comparison Tool")
root.geometry("500x250")
root.resizable(False, False)

tk.Label(root, text="File 1:").pack(pady=5)
entry_file1 = tk.Entry(root, width=60)
entry_file1.pack()
tk.Button(root, text="Browse...", command=browse_file1).pack()

tk.Label(root, text="File 2:").pack(pady=5)
entry_file2 = tk.Entry(root, width=60)
entry_file2.pack()
tk.Button(root, text="Browse...", command=browse_file2).pack()

tk.Button(root, text="Compare Files", command=compare_files, bg="blue", fg="white").pack(pady=10)
result_label = tk.Label(root, text="", font=("Arial", 12))
result_label.pack(pady=5)

root.mainloop()
