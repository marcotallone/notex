# import os

# # Specify the extensions of the files you want to delete
# extensions = [
#     ".aux", ".log", ".out", ".toc", ".lof", ".lot",
#     ".fls", ".blg", ".bbl", ".bcf", ".fdb_latexmk",
#     ".synctex.gz",".listing","-blx.bib", ".run.xml",
#     ".nav", ".snm", ".vrb", ".synctex.gz", ".synctex.gz(busy)"
# ]

# # Walk through the previous directory and its subdirectories
# for dirpath, dirnames, filenames in os.walk('.'):
#     # For each file in the current directory
#     for filename in filenames:
#         # If the file has one of the specified extensions
#         if any(filename.endswith(ext) for ext in extensions):
#             # Construct the full file path
#             file_path = os.path.join(dirpath, filename)
#             # Delete the file
#             os.remove(file_path)
#             print(f"Deleted {file_path}")

import os
import shutil

# Specify the extensions of the files you want to delete
extensions = [
    ".aux",
    ".log",
    ".out",
    ".toc",
    ".lof",
    ".lot",
    ".fls",
    ".blg",
    ".bbl",
    ".bcf",
    ".fdb_latexmk",
    ".synctex.gz",
    ".listing",
    "-blx.bib",
    ".run.xml",
    ".nav",
    ".snm",
    ".vrb",
    ".synctex.gz",
    ".synctex.gz(busy)",
]

# Specify the folder names you want to delete
folders_to_delete = ["_minted-main", "_minted-test"]

# Walk through the previous directory and its subdirectories
for dirpath, dirnames, filenames in os.walk("."):
    # For each file in the current directory
    for filename in filenames:
        # If the file has one of the specified extensions
        if any(filename.endswith(ext) for ext in extensions):
            # Construct the full file path
            file_path = os.path.join(dirpath, filename)
            # Delete the file
            os.remove(file_path)
            print(f"Deleted {file_path}")

    # For each directory in the current directory
    for dirname in dirnames:
        # If the directory is one of the specified folders to delete
        if dirname in folders_to_delete:
            # Construct the full directory path
            dir_path = os.path.join(dirpath, dirname)
            # Delete the directory and its contents
            shutil.rmtree(dir_path)
            print(f"Deleted directory {dir_path}")

