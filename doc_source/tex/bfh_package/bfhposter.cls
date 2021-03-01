% -*- mode: latex -*-
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{bfhletter}[2015/09/04 BFH Poster class]


% Optionprocessing START
% TODO Set length and design according to portrait or landscape option

% Pass all other Options to a0poster
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{a0poster}}
\ProcessOptions\relax
% Option Processing END
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\LoadClass{a0poster}

% BFH Style Packages
\RequirePackage{bfhcolors}
\RequirePackage{bfhfonts}
\RequirePackage{bfhlayout}


% Non BFH Packages
\RequirePackage{tikz}
\RequirePackage{multicol}
\RequirePackage{wrapfig}

% TODO: Customize a0poster for typesetting BFH posters.

% Define Head
\newcommand{\head}{}

% Define Foot
\newcommand{\foot}{%
}
