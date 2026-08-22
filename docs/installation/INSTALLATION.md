# Installation

The following document clarifies how to setup and install the NoTeX software and the LaTeX template.

## Requirements

The project requires the following dependencies installed in your system:

- CMake (>= 3.20)
- C++ compiler (C++20)
- [LaTeX distribution](https://www.latex-project.org/get/) installed, with the following packages:
  - `texlive-latex-base`
  - `texlive-latex-extra`
  - `texlive-fonts-recommended`
  - `texlive-fonts-extra`
  - `texlive-luatex`
  - `texlive-lang-english`
  - `texlive-bibtex-extra`
  - `texlive-science`
  - `biber`
  - `latexmk`
- [LuaLaTeX](https://www.luatex.org/) compiler: this can be installed with the above `texlive-luatex` package
- [Pygments](https://pygments.org/) python package: can be installed with `pip install Pygments` in your preferred environment

## CLI Utility Installation

### Building from source

The NoTeX software can be built from source following these steps:

1. Clone the repository:

   ```bash
   git clone https://github.com/marcotallone/notex.git
   ```

   and go at project's root:

   ```bash
   cd notex
   ```

2. Compile using cmake:

   ```bash
   mkdir -p build \
   && cmake -S . -B build  -DCMAKE_BUILD_TYPE=Release \
   && cmake --build build --parallel $(nproc)
   ```

   This will produce a unique binary in `./bin/notex`, containing the CLI utility with embedded LaTeX template.

3. _Optionally_, you can install the software system-wise for easier usage:

   ```bash
   sudo cmake --install build
   ```

   This will allow to use `notex` as a normal binary everywhere in your system.

   > [!WARNING]
   > The system-wise installation might require sudo priviledges!

4. Check if your installation was successful by running:

   ```bash
   notex info
   # or, if you skipped step 3:
   # ./bin/notex info
   ```

The [`install.sh`](../../install.sh) script provided in the project's repository helps to automate these steps with:

```bash
./install.sh # for local compilation only
```

or:

```bash
sudo ./install.sh global # for system-wise installation
```

> [!NOTE]
> Mind that the latter option also installs the LaTeX template system-wise, see belo for further details.

#### Build options

When building from source, the CMake compilation offers multiple options that are not set by default:

- `DEBUG`: enables debug mode, with extended logging reporting.
- `VERBOSE`: enables verbose mode.
- `LOGGING`: enables logging reporting. Logs are always stored in the `./log/notex.log` file in the project where `notex` is used.
- `TESTS`: enables tests. See [Testing](./Testing.md) for details.
- `COVERAGE`: enables coverage report. See [Testing](./Testing.md) for details.
- `DOCS`: allows to build project documentation providing the `docs` target. After setting this option with `-DDOCS=ON`, documentation can be built in `./build/docs/` with:

  ```bash
  cmake --build build --target docs
  ```

### Uninstall

You can uninstall a global `notex` installation with the command:

```bash
notex uninstall
```

Or, equivalently:

```bash
notex prune
```

A local installation can be analogously uninstalled by providing the path to the project installation:

```bash
notex uninstall <path>
```

## LaTeX Template Installation

After installing the `notex` CLI utility, the LaTeX template can be installed either **globally** (system-wise available) or **locally** (for the single LaTeX project).

### Global Installation

The template can be globally installed with:

```bash
notex install
```

Which will install the LaTeX classes and styles, as well as required fonts, into your TeX distribution's `TEXMFHOME` directory. To check which directory this is in your own system, you can run:

```bash
kpsewhich -var-value=TEXMFHOME
```

> [!TIP]
> Pass `--force` to overwrite an existing installation without being asked to confirm.

After a global installation te template can be loaded as:

```tex
\documentclass[OPTIONS]{notex}
```

### Local Installation

You can install the template locally, for yor project only, by running:

```bash
notex install <path>
```

where `<path>` is the path to your LaTeX project root directory. For instance if the root directory of your project is the current one run:

```bash
notex install .
```

The local installation will install the template and its classes in the `<path>/settings/` directory, while fonts will be placed in `<path>/fonts/`.

After a local installation the template can be loaded as:

```tex
\documentclass[OPTIONS]{settings/notex}
```

---

<div align="center">
<sub>
<b>NoTeX</b>
<br style="margin-bottom: 0.3rem;">

[Home](../README.md) &nbsp;•&nbsp; [Installation](./INSTALLATION.md) &nbsp;•&nbsp; [Usage](../usage/USAGE.md) &nbsp;•&nbsp; [Implementation](../implementation/IMPLEMENTATION.md)

</sub>
</div>
