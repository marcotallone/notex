# Naming Conventions

This document describes various naming conventions and preferences adopted as
consistently as possible throughout the codebase.

## C++ conventions

- `snake_case` for variables, function names, method names.
- `PascalCase` for class names.
- `SCREAMING_SNAKE_CASE` for constants and macros.
- Ending underscore for private member variables (e.g., `var_`).
- For paths (`std::string` or `fs::path`) append to the variable name:
  - `_dir` if the path is supposed to be a directory (`_dir_` if private member
    variable)
  - `_file` if the path is supposed to be a file (`_file_` if private member
    variable)
  - `_path` if the path can be either a file or a directory (or a priori
    unknown) (`_path_` if private member variable)
- Favor complete and descriptive names over short and cryptic ones (e.g.,
  `image` over `img`, `config` over `cfg`, `template` over `tmp`, etc.).
