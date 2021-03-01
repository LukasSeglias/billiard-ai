% -*- mode: latex -*-
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{bfhbook}[2015/09/04 BFH Book class]

% For handling conflicts with not yet defined commands, etc.
\RequirePackage{scrlfile}
% For if clauses
\RequirePackage{ifthen}

% Option Processing START
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Process Options created for bfhbook
% TODO
% Idea: Pass Type of bfhbook (Thesis, Lecture Notes, Project Report, etc. in
% order to set the titlepage, header and footer, etc.)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% thesis.
% sets the Titlepage, header and footer for a Thesis
\DeclareOption{thesis}{%
  \AfterClass{scrbook}{\renewcommand{\maketitle}{\@thesistitle}%
  % \ATEndOf.. is necessary in order to make scrbook commands like \maketitle
  % visible
}% End ``AfterClass''
}% End Option ``thesis''

% Lecture Notes
% sets the Titlepage, header and footer for lecture notes
% TO BE DEFINED

% bfhcolors
\DeclareOption{coated}{\PassOptionsToClass{coated}{bfhcolors}}

% Pass all other Options to scrbook
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{scrbook}}
\ProcessOptions\relax
% Option Processing END
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\LoadClass{scrbook}
% Pass DIV-Value here, not in the template!
\KOMAoptions{DIV=20,footinclude=on,headinclude=on}
% Include header and footer into the typearea.
% Otherwise header or footer could be outside paper
%\KOMAoptions{footinclude=on,headinclude=on}

% Loading BFH specific packages
\PassOptionsToPackage{table,hyperref}{xcolor}
\RequirePackage{bfhcolors}
\RequirePackage{bfhfonts}
\RequirePackage{bfhlayout}

% Language support
%\RequirePackage{babel}
\RequirePackage{translations}
\LoadDictionary{bfhtranslations}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Custom Titlepages for thesis, lecture notes, and?
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Definition of Titlepage for thesis
\makeatletter%
% Def von \@maketitle geht nicht! Warum?
\newcommand{\@thesistitle}{%
%  custom Titlepage. Option Processing redefines
% \maketitle according to the option
%%%
% Set pagestyle with BFH in the footline
  \pagestyle{BFHInFoot}% Pagestyles are defined in bfhlayout.sty
% TODO: Logo without text. Set in bfhlayout
  \includegraphics[height=18mm]{\Logo}%
\vspace{15mm}%

\@titlepicture%
\vspace{4ex}

{\Huge\textbf{\@title}}%
\vspace{2ex}

{\Large \@subtitle}%
\vspace{1ex}

\normalsize\textbf{\@type}% Bachelorthesis, Semesterproject, etc

\vfill

\begin{tabular}{@{}p{0.3\textwidth}p{0.6\textwidth}}%
% For Language support use \topicname, etc.
\GetTranslation{topic of study}: &\@degreeprogramme\\%\topicname
\GetTranslation{Author}: &\@author\\%\authorname
\GetTranslation{Supervisor}: &\@thesisadvisor\\%\supervisorname
% Line with Projectpartner misses if there is no
\ifthenelse{\equal{\@projectpartner}{}}{}{%
\GetTranslation{Project-Partner}: &\@projectpartner\\%\clientname
}% ENDIF
\GetTranslation{Expert}: &\@expert\\%\expertname
\GetTranslation{Date}: &\@date %\datename
\end{tabular}
\clearpage
%% Set usual pagestyle
  \pagestyle{BFHThesisBody}% Pagestyles are defined in bfhlayout.sty
}
\makeatother
% Titlepage for Thesis end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Set Font and Color for Captions
% Can this be moved to bfhfonts?

% \setkomafont{caption}{\bfseries\color{BFHGray}} % Move it to bfhbook.cls
\addtokomafont{caption}{\color{BFHGray}} % Move it to bfhbook.cls
\setkomafont{captionlabel}{\usekomafont{caption}}
\addtokomafont{footnote}{\color{BFHGray}}
\KOMAoption{captions}{belowfigure,abovetable,oneline}

% TODO: Customize scrbook for typesetting BFH books.
