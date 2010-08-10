"""
This is the place that takes the basic configuration of the LaTeX
build project.
"""

LATEX_PROJECT = 'manual'
DEFAULT_TARGET = 'pdf'

IMAGES_DIRECTORY = 'images'
GENERATED_DIRECTORY = 'generated'
CHAPTER_DIRECTORY = 'chapters'

FILE_EXTENSIONS = {'eps': '.eps',
                   'pdf': '.pdf',
                   'png': '.png',
                   'jpg': '.jpg',
                   'gnuplot': '.gnuplot',
                   'dot': '.dot'}

MAKEINDEX_EXTENSIONS = ['.glg', '.glo', '.gls']

