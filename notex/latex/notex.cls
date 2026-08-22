%┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓%
%┃                                                                            ┃%
%┃                         ░███                 ░███                          ┃%
%┃                        ░██     ░███    ░██     ░██                         ┃%
%┃                        ░██     ░████   ░██     ░██                         ┃%
%┃                        ░██     ░██░██  ░██     ░██                         ┃%
%┃                      ░███      ░██ ░██ ░██      ░██                        ┃%
%┃                        ░██     ░██  ░██░██     ░██                         ┃%
%┃                        ░██     ░██   ░████     ░██                         ┃%
%┃                        ░██     ░██    ░███     ░██                         ┃%
%┃                         ░███                 ░███                          ┃%
%┃                                                                            ┃%
%┃                               ~ N O T E X ~                                ┃%
%┃                      Copyright (c) 2026 Marco Tallone                      ┃%
%┃                                MIT License                                 ┃%
%┃                                                                            ┃%
%┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛%
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{notex}[A modern noteworthy LaTeX template. Version 1.0.0 (2026)]

% Runtime variable & conditional initializations
\newcommand{\notextheme}{light} % Default fallback theme
\newif\ifnotex@loadcallouts \notex@loadcalloutstrue
\newif\ifnotex@loadcode     \notex@loadcodetrue
% \newif\ifnotex@loadminted   \notex@loadmintedtrue % TODO: see notex-code todo

% Options interception declarations
\DeclareOption{light}{\renewcommand{\notextheme}{light}}
\DeclareOption{dark}{\renewcommand{\notextheme}{dark}}
\DeclareOption{tokyo}{\renewcommand{\notextheme}{tokyo}}
\DeclareOption{bw}{\renewcommand{\notextheme}{bw}}
\DeclareOption{nocallouts}{\notex@loadcalloutsfalse}
\DeclareOption{nocode}{\notex@loadcodefalse}
% \DeclareOption{nominted}{\notex@loadmintedfalse}

% Forward global document adjustments to base article engine
\DeclareOption*{ \PassOptionsToClass{\CurrentOption}{article} }
\ProcessOptions\relax

% Base engine class initialization
\LoadClass[a4paper, 11pt]{article}

% Theme
\input{notex-theme-\notextheme}

% Require core packages and settings
\input{notex-geometry}
\input{notex-general}
\input{notex-style}
\input{notex-fonts}
\input{notex-math}
\input{notex-figures}
\input{notex-tables}
\input{notex-tikz}
\input{notex-toc}

% Conditional feature modules integration
\ifnotex@loadcallouts
    \RequirePackage{notex-callouts}
\fi

\ifnotex@loadcode
    \RequirePackage{notex-code}
\fi

% Macros
\input{notex-macros}

\endinput
