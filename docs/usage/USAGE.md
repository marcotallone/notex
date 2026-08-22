# Usage

The following document shows how to use and work with the NoTeX CLI utility and the associated LaTeX template.

## Using the `notex` CLI Utility

Once installed (_See [installation](../installation/INSTALLATION.md) for more details_), the `notex` utility offers a set of useful commands to
facilitate managing and working with your LaTeX projects.

The following table summarized the main `notex` commands and their purpose. Use it as a quick reference. Further details can be obtained running the `notex help` (or `notex -h`) command.

| **Command**              | **Description**                                                        |
| :----------------------- | :--------------------------------------------------------------------- |
| `notex help`             | Shows `notex` binary options and help guide.                           |
| `notex version`          | Shows current binary version.                                          |
| `notex info`             | Displays installation and current project information.                 |
| `notex init .`           | Initializes a project in the current working directory.                |
| `notex clean <path>`     | Cleans recursively temporary files from the provided `<path>`.         |
| `notex get <snippet>`    | Provides a ready-to-use snippet LaTeX snippet template.                |
| `notex theme <theme>`    | Changes the current project theme.                                     |
| `notex add <what>`       | Adds a new section or bibliography (bib) to the current project.       |
| `notex remove <what>`    | Removes a section or bibliography (bib) from the current project.      |
| `notex ls`               | Lists the files that belong to the current project.                    |
| `notex checkhealth`      | Checks the TeX environment, the installation, and the project's state. |
| `notex set <key> <what>` | Changes a whitelisted `notex.json` key directly.                       |
| `notex reset`            | Regenerates `main.tex` from the template.                              |
| `notex delete`           | Removes notex files and build artefacts, keeps your documents.         |

The following guide explores the different set of operations that can be performed with `notex` in more detail.

> [!NOTE]
> This tutorial assume that the `notex` utility has been installed as a globally
> available binary.

### Initialization

You can quickly setup a LaTeX project that uses the NoTeX template in the current directory (_or in one of its sub-paths_) via the **init command**:

```bash
notex init .
# or equivalent:
# notex init multi .
```

The default project initialization process will start a **multi-file** project that uses the NoTeX template. This means that a `main.tex` using the `subfiles` package will be initialized and a `./sections/` folder will be available with sections files. A sample project tree layout will look as the following:

```bash

```

An example of this multi-file project can be found in the [examples directory](../../examples/multi/).

In alternative, for smaller projects, a **mono-file** project with a single `main.tex` file using the template can be initialized by specifying:

```bash
notex init mono .
```

An example of this mono-file project can be found in the [examples directory](../../examples/mono/).

Both commands will also create a `.notex/` directory at root level which contains useful NoTeX configurations and options.

> [!NOTE]
> `notex init` refuses to overwrite an existing `main.tex` or `.notex/` directory.<br>
> Pass `--force` to overwrite anyway.

In both cases, the generated `main.tex` automatically references `\documentclass{settings/notex}` when the target directory already holds a local installation, and `\documentclass{notex}` otherwise.

### Management

Once you have a working project, `notex` implements a series of management utility commands to facilitate certain operations on your project.

#### Change theme

You can change the theme used by the LaTeX template with:

```bash
notex theme <theme_name>
```

where `<theme_name>` can be one of the available themes:

- light (_default_)
- dark
- tokyo
- bw

<!-- TODO: screeshots of themes style examples and paste them here + complete TODO for "get themes" -->

#### Manage sections and bibliography

You can add a section by running:

```bash
notex add section "My Title"
```

which will simply add a section in case of a mono-file project or directly add a new file containing a section in case of a multi-file project.

Accordingly you can remove a section by its section number `<N>`:

```bash
notex remove section <N>
```

As for the sections, two analogous commands exist for biblography.

To create `bibliography.bib` and wire it into `main.tex`:

```bash
notex add bib
```

To remove bibliography:

```bash
notex remove bib
```

#### Cleaning

You LaTeX projects surely will fill up with temporary files and build artefacts. `notex` implements a command to clean up your project and remove all the temporary files generated by the LaTeX build process:

```bash
notex clean <path>
```

Cleans recursively temporary files from the provided `<path>`.

> [!WARNING]
> Cleaned files are **lost forever** once the command runs. If you want to check
> which files will be removed \*before actually removing them\*\*, please run a
> _"dry run"_ with:
>
> ```bash
> notex clean --dry-run
> ```
>
> This will list the files that will be removed by an eventual clean operation.

#### References and Snippets

You might need to get quick snippets references and usable LaTeX templates that the NoTeX theme implements in case you foget. `notex` implements a `get` command that can be used exactly for this reason:

```bash
notex get
```

Lists every ready-to-paste snippet name (_info, warning, note, tip, important, caution, cbox, ebox, code, ..._).
While you can get a ready-to-use LaTeX snippet template by specifying the name of the desired environment:

```bash
notex get box       # displays a template matching the "box" description
notex get warning   # displays a template for the warning callout
notex get code      # displays a template for the code environemnt
```

#### State and Info

Some commands allow to investigate and query the current state of the project or to set related properties.

```bash
notex ls
```

Lists the files that belong to the project (_main, sections, bibliography_).

```bash
notex checkhealth
```

Checks the TeX environment, the installation, and the project's state.

```bash
notex set main_file alt.tex
```

Changes a whitelisted `notex.json` key directly (_e.g., `main_file`, `theme`, `bibliography_file`, etc..._).

```bash
notex reset
```

Regenerates `main.tex` from the template, backing up the old one to `main.tex.bak`.

```bash
notex delete
```

Removes `.notex/`, `settings/`, `fonts/`, and build artefacts; keeps your documents untouched. If you want to destructively remove the whole project run:

```bash
notex delete --all
```

## Using the NoTeX template

The LaTeX template of the NoTeX project is directly available once `notex` installation is completed (_See [installation](../installation/INSTALLATION.md) for more details_).

Every NoTeX document loads the `notex` class instead of `article`:

```latex
\documentclass[light]{notex}
```

The bracketed option selects the theme, and the class itself pulls in a curated set of packages for fonts, math, figures, tables, TikZ diagrams, a styled table of contents, glossaries and acronyms, emoji, and FontAwesome icons, on top of two feature modules covered below: callout blocks and code blocks. Either module can be turned off with the `nocallouts`/`nocode` class options.

> [!NOTE]
> Compiling requires **LuaLaTeX** (needed for the FontAwesome icons) with `-shell-escape` enabled (code blocks are highlighted through **minted**, which shells out to `pygments`).

The rest of this section shows each feature in brief. For a complete, compiled showcase, see the [NoTeX documentation PDF](../NoTeX_Documentation.pdf), typeset from the [`examples/multi`](../../examples/multi/) project.

### Themes

Four built-in themes ship with the template, matching the ones `notex theme <name>` switches between on an initialized project:

| Theme   | `documentclass` option |
| :------ | :---------------------- |
| Light   | `light` (_default_)     |
| Dark    | `dark`                   |
| Tokyo   | `tokyo`                  |
| Black & white | `bw`               |

### Callout blocks

Colored and outlined boxes, admonitions, and math blocks highlight content outside the normal document flow:

```latex
\begin{cbox}
    A colored box.
\end{cbox}

\begin{warning}
    Something the reader should pay attention to.
\end{warning}

\begin{theorem}[Pythagorean Theorem][pythagorean]
    In a right triangle, $a^2 + b^2 = c^2$.
\end{theorem}
```

The full family includes `cbox`/`ebox` (colored/outlined boxes), `warning`, `info`/`blueinfo`, `note`, `tip`, `important`, `caution`, `example`, and the math blocks `definition`, `theorem`, `corollary`, `proposition`, `lemma`. Every one of them accepts an optional background/border color, and splits into an upper and lower part via `\tcblower`. Run `notex get <name>` for a ready-to-paste snippet of any of them.

### Code blocks

Code is highlighted through **minted**, with one environment per language plus a generic fallback:

```latex
\begin{python}[caption = {A greeting function}, label = {lst:greet}]
def hello(name):
    print(f"Hello, {name}!")
\end{python}
```

`caption` and `label` turn the block into a numbered, referenceable listing, just like a figure; `linenos`, `options`, and `tcb` control line numbers and let raw minted/tcolorbox settings through. `\cc{...}` and `\inline[lang]{...}` cover inline code, plain and highlighted respectively, and `pseudocode` is a dedicated environment for algorithms that need real math mode. `notex get code` prints a ready-to-paste template.

### Everything else

Math (`amsmath`, `mathtools`, `siunitx`, `cleveref`, ...), figures, tables, and TikZ diagrams all work exactly as in a normal `article`, styled to match the current theme. Acronyms are defined with `\newacronym` and used via `\gls`; `\emoji{name}` and `\faIcon{name}` insert emoji and FontAwesome icons respectively. A bibliography, added to a project with `notex add bib`, works with either `biblatex` or the classic `\bibliography` mechanism, whichever the project's `\usepackage` line for `biblatex` indicates.

---

<div align="center">
<sub>
<b>NoTeX</b>
<br style="margin-bottom: 0.3rem;">

[Home](../README.md) &nbsp;•&nbsp; [Installation](../installation/INSTALLATION.md) &nbsp;•&nbsp; [Usage](./USAGE.md) &nbsp;•&nbsp; [Implementation](../implementation/IMPLEMENTATION.md)

</sub>
</div>
