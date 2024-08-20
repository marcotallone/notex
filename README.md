<a name="readme-top"></a>

<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]
[![Gmail][gmail-shield]][gmail-url]

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/marcotallone/notex">
    <img src="images/pencil.png" alt="Logo" width="80" height="80">
  </a>

<h3 align="center">NoTeX</h3>

  <p align="center">
    A LaTeX template for organized and good-looking class notes.
    <br />
    <a href="./main.pdf"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="./main.tex">View Demo</a>
    ·
    <a href="https://github.com/marcotallone/notex/issues">Report Bug</a>
    ·
    <a href="https://github.com/marcotallone/notex/issues">Request Feature</a>
  </p>
</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>📑 Table of Contents</summary>
  <ol>
    <li>
        <a href="#news-and-updates">News and Updates 🚀</a>
    </li>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
  </ol>
</details>

<!-- NEWS -->
## News and Updates 🚀

Big News! Update version `2.0` is out! I've updated the documentation with all
the added features. This version remains retro-compatible with the previous
set-up at some level but you need to install [`pygments`](https://pygments.org/)
to take advantage of the new features. This can be done through the pip package
manager:

```
pip install pygments
```

The old version is still available in the `notex-1.0` side branch in case
something doesn't work. Enjoy!

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ABOUT THE PROJECT -->
## About The Project

<!-- [![Product Name Screen Shot][product-screenshot]](https://example.com) -->

Do you want to take organized and good-looking class notes? `NoTeX` is a LaTeX template that makes it easy to take notes in class. It's designed to be easy to use and to look great. It's also easy to customize to your liking and allows you to take advantage of LaTeX's powerful features.\
This template is designed to be used with the LaTeX typesetting system. It's a great way to take notes in class and keep them organized since it take advantage of latex's [subfiles](https://www.overleaf.com/learn/latex/Multi-file_LaTeX_projects) package in order to keep lecture notes and images in separate files, avoiding the common problem of having a single large file that is hard to navigate.\
Preamble setting also follows this philosophy: different settings are placed in easy-to-find separate files, so you can easily change the look of your notes without having to dig through thousands of preamble lines. The general structure of the template is as follows:

```bash
.
├── 📔 bibliography.bib   # Bibliography file
├── 📄 main.tex           # Main document file
├── 📂 images             # Images folder
│   └── image.png
├── 📂 sections           # Sections folder
│   ├── introduction.tex
│   ├── section.tex
│   └── ...
├── 📂 settings           # Settings folder
│   └── ...
└── 📂 utils              # Utilities
    └── ...
```

You will find then implemented NoTeX `cls` class and attached `sty` files in the `settings/` folder.\
This repository also provides a few uselful utility scripts in the `utils/` folder that will be updated with time. These might be helpful for stuff like cleaning up or collecting data from the tex files... Notice however that they are still a work in progress so check them out before blindly run them!

### Built With

![Neovim](https://img.shields.io/badge/Neovim-57A143?style=for-the-badge&logo=neovim&logoColor=white)
![Overleaf](https://img.shields.io/badge/Overleaf-47A248?style=for-the-badge&logo=overleaf&logoColor=white)
![LaTeX](https://img.shields.io/badge/LaTeX-008080?style=for-the-badge&logo=latex&logoColor=white)
![Python](https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white)
![VSCode](https://img.shields.io/badge/VSCode-007ACC?style=for-the-badge&logo=visual-studio-code&logoColor=white)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

Simply download this directory, open the `main.tex` file in your LaTeX editor, start a document with the `notex` document class as shown below and you're mostly ready to go.

```latex
\documentclass{settings/notex}
```

You can start your notes from the `main.tex` file by linking sections as shown in the [example provided](./main.tex) and add new sections in the `sections/` folder. If you change directories or file names, make sure to update the `tex` files accordingly.

### Prerequisites

>[!WARNING]
> This template **requires [LuaLaTeX](https://www.overleaf.com/learn/latex/LuaLaTeX)** to be compiled correctly. If you are using Overleaf, you can change the compiler in the settings. If you are using a local LaTeX editor, **you may need to install LuaLaTeX**.

This template also uses the **[fontawesome5](https://ctan.org/pkg/fontawesome5?lang=en)** package for icons. Other used packages can be found in the `cls` and `sty` files in the `settings/` folder.

If you want  to use this template without following these prerequisites, you might (*or might not, depends...*) be successful by simply commenting the unwanted packages in the `settings/` folder.

### Installation

1. Download this folder
2. Open the `main.tex` file in your LaTeX editor
3. Start taking beautiful notes!

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- USAGE EXAMPLES -->
## Usage

This template offers multiple **macros** and **environments** to keep you note organized and good-looking. You can find a detailed guide on how to use them together with some examples in the [`main.tex`](./main.tex) file and in the relative `sections/` files. Among the many features this template offers, it's worth mentioning the followings:

* code blocks and syntax highlighting
* definitions and theorems boxes
* example boxes
* info and warning boxes
* tikz pictures support
* and many more...

The `settings/` folder contains the settings for the template. These include packages imports and environments definition. More importantly, all the settings in the [`notex.cls`](./settings/notex.cls) class file have been extensively commented to allow for **user personalization**.

>[!NOTE]
> The template by default is set to a **dark** template but you can easily change to a **light** theme.

The template comes infact in a default dark theme color but all the colors are personalizable by modifying the color palettes in the [`notex.cls`](./settings/notex.cls) class file. Notice however that it has been implemented a default light theme if that suits your needs better. To use a light template you can just start your document with the `light` option as shown below:

```latex
\documentclass[light]{settings/notex}
```

Otherwise using the dark option will set the default dark theme.

```latex
\documentclass[dark]{settings/notex}
```

With the update `2.0` a new theme has been added: `tokyo`. You can use it as the others:

```latex
\documentclass[tokyo]{settings/notex}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTACT -->
## Contact

| Contact Me | |
| --- | --- |
| Mail | <marcotallone85@gmail.com> |
| LinkedIn | [LinkedIn Page](https://linkedin.com/in/marco-tallone-40312425b) |
| GitHub | [marcotallone](https://github.com/marcotallone) |

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* [Best-README-Template](https://github.com/othneildrew/Best-README-Template?tab=readme-ov-file)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[forks-shield]: https://img.shields.io/github/forks/marcotallone/notex.svg?style=for-the-badge
[forks-url]: https://github.com/marcotallone/notex/network/members
[stars-shield]: https://img.shields.io/github/stars/marcotallone/notex.svg?style=for-the-badge
[stars-url]: https://github.com/marcotallone/notex/stargazers
[issues-shield]: https://img.shields.io/github/issues/marcotallone/notex.svg?style=for-the-badge
[issues-url]: https://github.com/marcotallone/notex/issues
[license-shield]: https://img.shields.io/github/license/marcotallone/notex.svg?style=for-the-badge
[license-url]: https://github.com/marcotallone/notex/blob/master/LICENSE.txt
<!-- [linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/marco-tallone-40312425b -->
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-blue?style=for-the-badge&logo=linkedin&logoColor=white&colorB=0077B5
[linkedin-url]: https://linkedin.com/in/marco-tallone-40312425b
<!-- [gmail-shield]: https://img.shields.io/badge/-Gmail-black.svg?style=for-the-badge&logo=gmail&colorB=555
[gmail-url]: mailto:marcotallone85@gmail.com -->
[gmail-shield]: https://img.shields.io/badge/-Gmail-red?style=for-the-badge&logo=gmail&logoColor=white&colorB=red
[gmail-url]: mailto:marcotallone85@gmail.com

