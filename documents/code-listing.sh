#!/usr/bin/env bash

tex_file=$(mktemp) ## Random temp file name

cat<<EOF >$tex_file   ## Print the tex file header
\documentclass{article}
\usepackage{listings}
\usepackage[usenames,dvipsnames]{color}  %% Allow color names     
\usepackage[colorlinks=true,linkcolor=blue]{hyperref} 
\begin{document}
\tableofcontents

EOF

# steering

echo "
  \lstdefinestyle{customasm}{
    language=C,
    belowcaptionskip=1\baselineskip,
    xleftmargin=\parindent,
    breaklines=true, %% Wrap long lines
    basicstyle=\footnotesize\ttfamily,
    commentstyle=\itshape\color{Gray},
    stringstyle=\color{Black},
    keywordstyle=\bfseries\color{OliveGreen},
    identifierstyle=\color{blue},
    xleftmargin=-8em,
  }
" >> $tex_file &&

find ../steering -name "*\.c" -o -name "\.h" |

while read  i; do                ## Loop through each file
    name=${i//_/\\_}             ## escape underscores
    name=$(echo "$name" | sed 's/^\...//')
    echo "\newpage" >> $tex_file   ## start each section on a new page
    echo "\section{$name}" >> $tex_file  ## Create a section for each filename

   ## This command will include the file in the PDF
    echo "\lstinputlisting[style=customasm]{$i}" >>$tex_file
done &&


# sensor

echo "
  \lstdefinestyle{customasm}{
    language=C,
    belowcaptionskip=1\baselineskip,
    xleftmargin=\parindent,
    breaklines=true, %% Wrap long lines
    basicstyle=\footnotesize\ttfamily,
    commentstyle=\itshape\color{Gray},
    stringstyle=\color{Black},
    keywordstyle=\bfseries\color{OliveGreen},
    identifierstyle=\color{blue},
    xleftmargin=-8em,
  }
" >> $tex_file &&

find ../sensor -name "*\.c" -o -name "\.h" |

while read  i; do                ## Loop through each file
    name=${i//_/\\_}             ## escape underscores
    name=$(echo "$name" | sed 's/^\...//')
    echo "\newpage" >> $tex_file   ## start each section on a new page
    echo "\section{$name}" >> $tex_file  ## Create a section for each filename

   ## This command will include the file in the PDF
    echo "\lstinputlisting[style=customasm]{$i}" >>$tex_file
done &&


# communication

echo "
  \lstdefinestyle{customasm}{
    language=C++,
    belowcaptionskip=1\baselineskip,
    xleftmargin=\parindent,
    breaklines=true, %% Wrap long lines
    basicstyle=\footnotesize\ttfamily,
    commentstyle=\itshape\color{Gray},
    stringstyle=\color{Black},
    keywordstyle=\bfseries\color{OliveGreen},
    identifierstyle=\color{blue},
    xleftmargin=-8em,
  }
" >> $tex_file &&

find ../communication/src -name "*\.cpp" -o -name "\.hpp" |

while read  i; do                ## Loop through each file
    name=${i//_/\\_}             ## escape underscores
    name=$(echo "$name" | sed 's/^\...//')
    echo "\newpage" >> $tex_file   ## start each section on a new page
    echo "\section{$name}" >> $tex_file  ## Create a section for each filename

   ## This command will include the file in the PDF
    echo "\lstinputlisting[style=customasm]{$i}" >>$tex_file
done &&

# pc

echo "
  \lstdefinestyle{customasm}{
    language=Python,
    belowcaptionskip=1\baselineskip,
    xleftmargin=\parindent,
    breaklines=true, %% Wrap long lines
    basicstyle=\footnotesize\ttfamily,
    commentstyle=\itshape\color{Gray},
    stringstyle=\color{Black},
    keywordstyle=\bfseries\color{OliveGreen},
    identifierstyle=\color{blue},
    xleftmargin=-8em,
  }
" >> $tex_file &&

find ../pc -name "*\.py" |

while read  i; do                ## Loop through each file
    name=${i//_/\\_}             ## escape underscores
    name=$(echo "$name" | sed 's/^\...//')
    echo "\newpage" >> $tex_file   ## start each section on a new page
    echo "\section{$name}" >> $tex_file  ## Create a section for each filename

   ## This command will include the file in the PDF
    echo "\lstinputlisting[style=customasm]{$i}" >>$tex_file
done &&

echo "\end{document}" >> $tex_file &&
pdflatex $tex_file -output-directory . && 
pdflatex $tex_file -output-directory .  ## This needs to be run twice 
                                           ## for the TOC to be generated  