#!/usr/bin/python
#
# TODO:
# - Task colors:
#     - User-defined using config file.
#     - Automagically chosen from color space.
#     - Advanced algorithm (contact Hannes Pretorius).
# - Koos' specs:
#     - Resources and tasks sorted in read-in order (default) 
#       or alphabetically (flag).
#     - Have proper gnuplot behavior on windows/x11, eps/pdf, latex terminals.
#     - Create and implement algorithm for critical path analysis.
# - Split generic stuff into a Gantt class, and specific stuff into the main.
#
# gantt.py ganttfile | gnuplot

# Find out if we're using the set built-in type (Python > 2.3)
# or the Set module (Python <= 2.3)
try:
    set
except NameError:
    from sets import Set as set
from itertools import *
from optparse import OptionParser
from ConfigParser import ConfigParser

class Activity:
    """ Container for activity information
    """
    def __init__(self, resource, start, stop, task):
        self.resource = resource
        self.start = start
        self.stop = stop
        self.task = task

class Rectangle:
    """ Container for rectangle information
    """
    def __init__(self, bottomleft, topright, fillcolor):
        self.bottomleft = bottomleft
        self.topright = topright
        self.fillcolor = fillcolor
        self.fillstyle = 'solid 0.8'
        self.linewidth = 2

class ColorBook:
    """ 
    """
    def __init__(self, tasks, filename=None, palette=None):
        if palette == 'palette':
            self._colors_palette(tasks, filename)
        elif palette == 'config':
            self._colors_defined(tasks, filename)
        else:
            self._colors_palette(tasks, filename)

    def _colors_palette(self, tasks, palettefile):
        # Palette settings for task colors as fractions from palette
        self.palette = 'set palette model RGB ' + \
                       'file "%s" using ($1/255):($2/255):($3/255)' % palettefile
        self.prefix = 'palette frac'
        # Divide colorspace into fractions, one per task
        fracs = ['%0.3f' % x * (1.0 / len(tasks) - 1) 
                 for x in xrange(len(tasks))]
        self.colors = dict(izip(tasks, fracs))
    
    def _colors_config(self, tasks, colorfile):
        # Palette settings for user-defined task colors
        self.palette = 'set palette model RGB'
        self.prefix= 'rgb'
        # Read in task colors from configuration file
        config = ConfigParser()
        config.optionxform = str # makes option names case sensitive
        config.readfp(open(colorfile, 'r'))
        # Colors are RGB colornames
        self.colors = dict(config.items('Colors'))
        # Raise KeyError if no color is specified for a task
        nocolors = [t for t in tasks if not self.colors.has_key(t)]
        if nocolors:
            errorMsg = 'Could not find task color for ' \
                       + ', '.join(nocolors)
            raise KeyError(errorMsg)
       

if __name__ == '__main__':
    # Option parser
    usage = 'usage: %prog [options] file'
    version = 'unsupported draft'
    parser = OptionParser(usage=usage, version='%prog version ' + version)
    # Add option for output file
    parser.add_option('-o', '--output',
                      type='string', dest='outputfile',
                      metavar='FILE',
                      help='Write output to FILE')
    # Add option for using Gnuplot palette file
#    parser.add_option('-p', '--palette',
#                      type='string', dest='palettefile',
#                      metavar='FILE',
#                      help='Pick task colors (RGB) from a Gnuplot palette specified in FILE (see Gnuplot documentation')
    # Add option for task color mapping
    parser.add_option('-c', '--color',
                      type='string', dest='colorfile',
                      metavar='FILE',
                      help='Use task colors (RGB) as defined in configuration FILE (in RGB triplets, Gnuplot colornames, or hexadecimal representations)')
    # Add switch for alphabetical sort of resources and tasks
    parser.add_option('-a', '--alpha',
                      action='store_true', dest='alphasort',
                      default=False,
                      help='Show resources and tasks in alphabetical order')
    # Populate parser
    # Add option for plot title
    parser.add_option('-t', '--title',
                      type='string', dest='plottitle',
                      metavar='TITLE',
                      default='',
                      help='Set plot title to TITLE (between double quotes)')
    # Add option for xtics
    parser.add_option('-x', '--xtics',
                      type='string', dest='xtics',
                      default='1',
                      help='Set xtics increment')
    # Add option for xlabel
    parser.add_option('-l', '--xlabel',
                      type='string', dest='xlabel',
                      default='time',
                      help='xlabel...')
    (options, args) = parser.parse_args()
    # Check if correct number of arguments is supplied
    if len(args) != 1:
        parser.error('incorrect number of arguments')
    # Single argument: file with gantt data
    ganttfile = args[0]

    # Read in activities from ganttfile: resource start stop task
    activities = []
    for line in open(ganttfile, 'r').readlines():
        line = line.split()
        resource = line[0]
        start = eval(line[1])
        stop = eval(line[2])
        task = line[3]
        activities.append(Activity(resource, start, stop, task))

    # Create list with unique resources and tasks in activity order.
    resources = []
    tasks = []
    for a in activities:
        if a.resource not in resources:
            resources.append(a.resource)
        if a.task not in tasks:
            tasks.append(a.task)

    # Sort such that resources and tasks appear in alphabetical order
    if options.alphasort:
        resources.sort()
        tasks.sort()

    # Resources are read from top (y=max) to bottom (y=1)
    resources.reverse()

    # Assign indices to resources and tasks
    resourceID = dict(izip(resources, count(1)))
    taskID = dict(izip(tasks, count(1)))
    
    # Set task colors
    # SE colors (see http://w3.wtb.tue.nl/nl/organisatie/systems_engineering/info_for_se_students/how2make_a_poster/pictures/)
    # Decrease the 0.8 values for less transparent colors.
    se_palette = {"se_red": (1.0, 0.8, 0.8),
                 "se_pink": (1.0, 0.8, 1.0),
                 "se_violet": (0.8, 0.8, 1.0),
                 "se_blue": (0.8, 1.0, 1.0),
                 "se_green":(0.8, 1.0, 0.8),
                 "se_yellow": (1.0, 1.0, 0.8)}
    se_gradient = ["se_red", "se_pink", "se_violet",
                   "se_blue", "se_green", "se_yellow"]
    se_palettedef = '( ' + \
                    ', '.join(('%d ' % n + 
                               ' '.join((str(x) for x in se_palette[c])) 
                               for n, c in enumerate(se_gradient))) + \
                    ' )'
    # Use RGB colors from user input
    # Use gnuplot> set palette color model RGB
    # and gnuplot> show palette colornames for RGB color names
    if options.colorfile:
        palettedef = 'model RGB'
        colorprefix = 'rgb'

        # Read in task colors from configuration file
        config = ConfigParser()
        config.optionxform = str # makes option names case sensitive
        config.readfp(open(options.colorfile, 'r'))
        # Colors are RGB colornames
        colors = dict(config.items('Colors'))

        # Raise KeyError if no color is specified for a task
        nocolors = [t for t in tasks if not colors.has_key(t)]
        if nocolors:
            errorMsg = 'Could not find task color for ' + ', '.join(nocolors)
            raise KeyError(errorMsg)

    # Pick colors from a pre-defined palette
    else:
        palettedef = 'model RGB defined %s' % se_palettedef
        colorprefix = 'palette frac'
        # Colors are fractions from the palette defined
        colors = dict([(t, '%0.2f' % (float(n)/(len(tasks)-1))) for n, t in enumerate(tasks)])

    # Create rectangle objects
    rectangleHeight = 0.8
    rectangles = []
    for a in activities:
        ypos = resourceID[a.resource]
        bottomleft = (a.start, ypos - 0.5 * rectangleHeight)
        topright = (a.stop, ypos + 0.5 * rectangleHeight)
        fillcolor = colors[a.task]
        rectangles.append(Rectangle(bottomleft, topright, fillcolor))

    # Plot dimensions
    xmin = 0
    xmax = max((a.stop for a in activities))
    ymin = 0 + (rectangleHeight / 2)
    ymax = len(resources) + 1 - (rectangleHeight / 2)
    xlabel = options.xlabel
    ylabel = ''
    title = options.plottitle
    xtics = options.xtics
    ytics = ''.join(['(',
                     ', '.join((('"%s" %d' % item) 
                                for item in resourceID.iteritems())),
                     ')'])
    #keyPosition = 'outside width +2' # outside and 2 characters from the graph
    keyPosition = 'nobox out horiz center top'

    gridTics = 'xtics'

    # Set plot dimensions
    plot_dimensions = ['set xrange [%f:%f]' % (xmin, xmax),
                       'set yrange [%f:%f]' % (ymin, ymax),
                       'set autoscale x', # extends x axis to next tic mark
                       'set xlabel "%s"' % xlabel,
                       'set ylabel "%s"' % ylabel,
                       'set title "%s"' % title,
                       'set ytics %s' % ytics,
 		       'set xtics %s' % xtics,
 		       'set key %s' % keyPosition,
                       'set grid %s' % gridTics,
                       'set palette %s' % palettedef,
			'unset colorbox']

    # Generate gnuplot rectangle objects
    plot_rectangles = (' '.join(['set object %d rectangle' % n,
                                 'from %f, %0.1f' % r.bottomleft,
                                 'to %f, %0.1f' % r.topright,
                                 'fillcolor %s %s' % (colorprefix, r.fillcolor),
                                 'fillstyle solid 0.8'])
                       for n, r in izip(count(1), rectangles))

    # Generate gnuplot lines
    plot_lines = ['plot ' + 
                  ', \\\n\t'.join((' '.join(['-1',
                                       'title "%s"' % t, 
                                       'with lines', 
                                       'linecolor %s %s ' % (colorprefix, colors[t]), 
                                       'linewidth 6']) 
                            for t in tasks))]

    if options.outputfile:
        g = open(options.outputfile, 'w')
        g.write('\n'.join(chain(plot_dimensions, plot_rectangles, plot_lines)))
        g.close()
    else:
        print '\n'.join(chain(plot_dimensions, plot_rectangles, plot_lines))
