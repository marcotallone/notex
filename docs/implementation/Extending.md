# Extending NoTeX

The boundaries between the components are drawn so that the common extensions stay small. This page walks through four of them: adding a theme or a fragment to the LaTeX template, adding a snippet to `notex get`, adding a key to the project metadata, and adding a command. It assumes you have read the [Architecture](./Architecture.md) page and can build the project as described in the [installation guide](../installation/INSTALLATION.md).

## Where New Logic Belongs

One rule decides where a new piece of code goes: if it would still make sense behind a different interface, it belongs in `notex_core`, and if it only makes sense as something typed on a terminal, it belongs in the `Orchestrator`.

Argument parsing, the shape of a message, and the choice of exit code are frontend concerns. Everything about what a project _is_ and what an operation _does_ to it belongs in the core library, where a future graphical frontend could reuse it unchanged. The `Orchestrator` calling a one-line `Manager` method is the intended shape, and logic that accumulates in a `run_*()` method is a sign that a domain method is missing.

Every component has a matching test file under [`notex/tests/`](../../notex/tests/), one per component, so an extension normally comes with a test in the file that already covers its component. The [testing page](../installation/Testing.md) describes how the suite is built and run.

## Adding to the Template

### A New Theme

A theme is one isolated file. Copy an existing one, for instance [`notex-theme-dark.tex`](../../notex/latex/notex-theme-dark.tex), and redefine the same colours:

```bash
cp notex/latex/notex-theme-dark.tex notex/latex/notex-theme-solarized.tex
```

Then declare the option in [`notex.cls`](../../notex/latex/notex.cls), next to the themes already there:

```tex
\DeclareOption{solarized}{\renewcommand{\notextheme}{solarized}}
```

Rebuilding re-embeds the new file, and `Manager::available_themes()` picks the name up from it automatically, since the list of themes is derived from the embedded `notex-theme-*.tex` file names rather than written out in the C++ source. No other file changes, and `notex theme solarized` starts working.

> [!WARNING]
> Both steps are required, and skipping the second fails quietly. The C++ side accepts the name as soon as the file exists, so `notex theme solarized` would happily write `\documentclass[solarized]{notex}`, while LaTeX would pass the undeclared option through to `article` and keep rendering the default theme.

### A New Fragment

Fragments are loaded by the class driver in a fixed order. Add the file to `notex/latex/` with the mandatory `notex-` prefix, keep it flat, and add its `\input` line to `notex.cls` at the position its dependencies require:

```tex
\input{notex-geometry}
\input{notex-general}
\input{notex-glossary}   % new fragment
\input{notex-style}
```

Load order is how package option clashes are avoided, so a fragment that configures a package another fragment also loads has to come first, or pass its options through `\PassOptionsToPackage` in the driver. The reasons behind the flat layout and the prefix are on the [LaTeX Template Integration](./LaTeXIntegration.md) page.

## Adding a Snippet

Snippets live in the table inside [`reference.cpp`](../../notex/src/notex/reference.cpp). Each entry is a canonical name, a list of aliases, and the text to print:

```cpp
{"remark",
 {"note"},
 "\\begin{remark}[OPTIONAL_TITLE]\n    WRITE_HERE\n\\end{remark}\n"},
```

Aliases are looked up exactly like canonical names, and one name may resolve to several snippets, which is how `notex get box` returns both box environments. Write placeholders in capitals, matching the existing entries, so that anything left unreplaced stands out in the compiled document.

The table mirrors the environments defined in the style files by hand, so add the entry in the same change as the environment itself. Nothing checks that the two agree.

## Adding a Configuration Key

A metadata key touches three places in [`manager.cpp`](../../notex/src/notex/manager.cpp) and one in [`manager.hpp`](../../notex/include/notex/manager.hpp):

1. A field on `ProjectConfig`, defaulting to an empty string unless it has a meaningful default.
2. A line in `load_config()`, reading it with a fallback so that a file written before the key existed still loads.
3. A line in `write_config()`, so the key is always emitted.
4. An entry in `set_config_value()`'s whitelist, but only if a user should be able to set it directly.

Adding a field this way is backward compatible in both directions. An older `notex.json` loads because the read falls back to the default, and a newer one is understood by older code because unknown keys are ignored. `schema_version` exists for the changes that are not backward compatible, such as reinterpreting a field that already has a meaning.

> [!NOTE]
> The whitelist in `set_config_value()` is a deliberate boundary, not an oversight. `notex set` writes metadata and never touches your documents, so exposing a key there is only appropriate when changing the record alone is a coherent operation. A key that must stay consistent with the content of `main.tex`, the way `theme` does, needs a command of its own that updates both.

## Adding a Command

A command is a registration, a piece of backing storage, and a method. In [`orchestrator.hpp`](../../notex/include/orchestrator.hpp), declare the private `run_*()` method and the members CLI11 will parse into. In `register_commands()`, declare the command itself:

```cpp
auto* rename_command = app_.add_subcommand(
    "rename", "Rename the project's main file");
rename_command->add_option("name", rename_name_, "New file name")
    ->required();
rename_command->callback([this] { run_rename(); });
```

The `run_*()` method should stay at the level of the example commands already there: construct the domain object, call one method on it, report the outcome through `ui`, and let any exception travel up to the single handler in `run()`. Raise `UsageError` for an argument the parser accepted but the command cannot use, so the process exits with the usage code.

> [!TIP]
> Pick positional argument names freely, but remember that a positional value matching another subcommand's name is only handled correctly because the parser is capped at one subcommand per invocation. That cap is set at the top of `register_commands()` and should stay there. The [CLI page](./CLIErrorsDiagnostic.md) explains what it prevents.

If the command needs new behaviour rather than a new arrangement of existing behaviour, add that behaviour as a method on `Manager` or `Installer` first, and let the command call it. That keeps the frontend thin and keeps the new capability available to anything else built on `notex_core`.

---

<div align="center">
<sub>
<b>NoTeX</b>
<br style="margin-bottom: 0.3rem;">

[Home](../README.md) &nbsp;•&nbsp; [Installation](../installation/INSTALLATION.md) &nbsp;•&nbsp; [Usage](../usage/USAGE.md) &nbsp;•&nbsp; [Implementation](./IMPLEMENTATION.md)

</sub>
</div>
