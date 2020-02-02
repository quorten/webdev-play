#! /usr/bin/env python
# Plot some time-series log data using Matplotlib, and make it easily
# accessible via a web CGI script.

# TODO: Add Bokeh support.
# 20181223/https://bokeh.pydata.org/en/latest/docs/user_guide/quickstart.html#userguide-quickstart

# JJJ TODO: UPDATE Raspberry Pi to Stretch.
# 20181223/https://www.raspberrypi.org/documentation/raspbian/updating.md

import sys
import time
import dateutil.parser
from dateutil.relativedelta import relativedelta
import datetime
from optparse import OptionParser

import sys
import matplotlib
# matplotlib.use('agg')
# matplotlib.use('svg')
import matplotlib.dates
import matplotlib.pyplot as plt
import numpy as np

DMY_HOUSE_LOG_FILE = '/home/www/dmy_house_logs.log'

PERSONS = ['Andrew', 'Anna', 'Chris', 'Dad', 'Mom', 'Nadia']

opt_parser = OptionParser()
opt_parser.add_option('-z', '--zone', dest='temper_zone', default=3,
                      type='int',
                      help='Temperature zone to plot (default 0)')
opt_parser.add_option('-r', '--smooth-radius', dest='smooth_rad', default=3,
                      type='int',
                      help='Smoothing radius for temperature data (default 3)')
opt_parser.add_option('-d', '--num-days', dest='num_days', default=0,
                      type='int',
                      help='Number of days history to display')
opt_parser.add_option('-f', '--plot-fmt', dest='plot_fmt', default='o',
                      help='Matplotlib format string to use for plots '
                      "(i.e. '.', 'o', '-')")
opt_parser.add_option('-c', '--cgi-stdout', dest='cgi_stdout',
                      action='store_true',
                      help='Output as CGI PNG on standard output')
opt_parser.add_option('-o', '--output-file', dest='output_file', default='',
                      help='File name to write image to')
opt_parser.add_option('-t', '--output-type', dest='output_type', default='svg',
                      help='Output format to use (svg, png)')
opt_parser.add_option('-m', '--data-mode', dest='data_mode', default=0,
                      type='int',
                      help='Select the data plotting mode.\n'
                           '0: Temperature plot mode.\n'
                           '1: Dish washing plot mode.\n'
                           '2: Food supply plot mode.\n'
                           '3: Clothing washing plot mode.\n'
                           '4: Laundry reservation plot mode.\n'
                           '5: Billing plot mode.\n')

(options, args) = opt_parser.parse_args()
temper_zone = options.temper_zone
smooth_rad = options.smooth_rad
num_days = options.num_days
fmt_str = options.plot_fmt
cgi_stdout = options.cgi_stdout
output_file = options.output_file
output_type = options.output_type
data_mode = options.data_mode

# Save the latest date.
latest_date = None

all_plots = []

#if temper_zone == 0:
#    TEMPER_LOG_FILE = 'temper/zone0.log'
#else:
#    TEMPER_LOG_FILE = 'temper/zone1.log'

if data_mode == 0:
    PT_PER_DAY = 288
if data_mode == 1 or data_mode == 2:
    PT_PER_DAY = 10

# Read the data into the correct data structures for plotting.

if data_mode == 0:
    date = None
    for TEMPER_LOG_FILE in ['temper/zone0.log', 'temper/zone1.log']:
        with open(TEMPER_LOG_FILE) as f:
            x = []
            y = []
            for line in f:
                date_temp = line.rstrip().split(': ')
                date = dateutil.parser.parse(date_temp[0])
                x.append(matplotlib.dates.date2num(date))
                temp = float(date_temp[1][5:-2]) # temp=29.9'C
                # Note: My Raspberry Pi B+ runs approximately 8 degrees
                # Celsius higher than room temperature.
                if TEMPER_LOG_FILE == 'temper/zone0.log':
                    temp = (temp - 8) * 9 / 5 + 32
                elif TEMPER_LOG_FILE == 'temper/zone1.log':
                    temp = (temp - 14.3125) * 9 / 5 + 32
                y.append(temp)

        # Now with temperature data, we need to smooth out the data
        # with a windowing function.  The temperature data comes in a
        # stair-step fashion, but the frequency at adjacent steps can
        # be used to determine sub-step precise temperature values.
        # Plus, without the smoothing, we otherwise get too much
        # noise.

        # We smooth with a window size = 10 points.  Take the 5
        # trailing and 5 leading points at a specific spot and average
        # them together.  At the edges, simply extrapolate the edge
        # value.
        if smooth_rad > 0:
            smooth_y = []
            i = 0
            while i < len(x):
                accum = 0
                j = 0
                while j < smooth_rad * 2:
                    sub_idx = i + j - smooth_rad
                    if sub_idx < 0:
                        sub_idx = 0
                    elif sub_idx >= len(x):
                        sub_idx = len(x) - 1
                    accum += y[sub_idx]
                    j += 1
                accum /= smooth_rad * 2
                smooth_y.append(accum)
                i += 1

            y = smooth_y
        all_plots.append([x, y])
    # Save the latest date.
    latest_date = date

elif data_mode in (2,3,4):
    date = None
    all_plots = [[[], []] for person in PERSONS]
    all_plots.append([[], []])
    old_x_val = 0
    y_val = 0
    washer_in_use = False
    dryer_in_use = False
    pre_checkout = False
    with open(DMY_HOUSE_LOG_FILE) as f:
        for line in f:
            old_y_val = y_val
            do_point = False
            record = line.rstrip().split(': ')
            date = dateutil.parser.parse(record[0])
            if data_mode == 1:
                if record[2] == 'Dining':
                    if record[3] == 'Meal':
                        y_val += 4
                        do_point = True
                    elif record[3] == 'Pack lunch':
                        y_val += 2
                        do_point = True
                    elif record[3] == 'Snack':
                        y_val += 1
                        do_point = True
                elif record[2] == 'Cooking':
                    # Cooking produces leftover containers, which will
                    # later be emptied and washed.
                    if record[3] == 'Check-out (meta-action, collect waste, clean cookware)':
                        y_val += 1
                        do_point = True
                    elif record[3] == 'Microwave cook food':
                        y_val += 1
                        do_point = True
                if record[2] == 'Dishes' and record[3] == 'Start dishwasher':
                    y_val -= 26
                    do_point = True
            elif data_mode == 2:
                if record[2] == 'Dining':
                    if record[3] == 'Meal':
                        y_val -= 4
                        do_point = True
                    if record[3] == 'Pack lunch':
                        y_val -= 4
                        do_point = True
                    if record[3] == 'Snack':
                        y_val -= 1
                        do_point = True
                if (record[2] == 'Pantry/refrigerator' and
                    record[3] == 'Check-in (meta-action, shopping, '
                                 'insert stock)'):
                    if record[1] == 'Dad':
                        y_val += 150
                        do_point = True
                    elif record[1] == 'Nadia':
                        y_val += 20
                        do_point = True
            elif data_mode == 3:
                if record[2] == 'Laundry':
                    if record[3] == 'Empty washer':
                        x_val = matplotlib.dates.date2num(date)
                        if old_x_val == 0:
                            old_x_val = x_val
                            # Choose some reasonable starting baseline.
                            y_val = 3.5 * 3.25 / 2
                        # One outfit change per day.
                        # Two loads of laundry per person per week.
                        # Count a fractional person for Anna because Dad
                        # sometimes finishes Anna's loads.
                        y_val += (x_val - old_x_val) * 3.25
                        old_y_val = y_val
                        y_val -= 3.5
                        do_point = True
            elif data_mode == 4:
                if record[2] == 'Laundry':
                    if record[3] == 'Check-in (meta-action, start first load)':
                        y_val += 1
                        do_point = True
                    elif record[3] == 'Start washer':
                        washer_in_use = True
                        y_val += 1
                        do_point = True
                    elif record[3] == 'Start dryer':
                        dryer_in_use = True
                        y_val += 1
                        do_point = True
                    elif record[3] == 'Empty washer' and not pre_checkout:
                        washer_in_use = False
                        y_val -= 1
                        do_point = True
                    elif record[3] == 'Empty dryer':
                        dryer_in_use = False
                        y_val -= 1
                        do_point = True
                    elif record[3] == 'Pre-check-out (meta-action, last load washed, washer ready)':
                        pre_checkout = True
                        if washer_in_use:
                           washer_in_use = False
                           y_val -= 1
                        do_point = True
                    elif record[3] == 'Check-out (meta-action, last load dried, dryer ready)':
                        pre_checkout = False
                        y_val -= 1
                        if washer_in_use:
                            washer_in_use = False
                            y_val -= 1
                        if dryer_in_use:
                            dryer_in_use = False
                            y_val -= 1
                        do_point = True
            if not do_point:
                continue
            x_val = matplotlib.dates.date2num(date)
            old_x_val = x_val
            # Append the old y_val at the current date to get a nice
            # stair-step effect.
            all_plots[len(PERSONS)][0].append(x_val)
            all_plots[len(PERSONS)][1].append(old_y_val)

            # Undefined without more detailed context.
            # all_plots[len(PERSONS)][0].append(x_val)
            # all_plots[len(PERSONS)][1].append(1)
            all_plots[len(PERSONS)][0].append(x_val)
            all_plots[len(PERSONS)][1].append(y_val)
            all_plots[PERSONS.index(record[1])][0].append(x_val)
            all_plots[PERSONS.index(record[1])][1].append(y_val)
    latest_date = date

if data_mode in (2,3,4):
    for person in PERSONS:
        graphs = all_plots[PERSONS.index(person)]
        plt.plot_date(graphs[0], graphs[1], 'o')
    graphs = all_plots[len(PERSONS)]
    plt.plot_date(graphs[0], graphs[1], '-')
    plt.legend(['Andrew', 'Anna', 'Chris', 'Dad', 'Mom', 'Nadia', 'Total'])
    if data_mode == 1:
        plt.title('Meals versus dish washing')
    elif data_mode == 2:
        plt.title('Meals versus shopping')
    elif data_mode == 3:
        plt.title('Outfit changes per person versus laundry')
    elif data_mode == 4:
        plt.title('Laundry reservations per person')
else:
    for item in all_plots:
        plt.plot_date(item[0], item[1], fmt_str)
    if data_mode == 0:
        plt.title('Zone temperature logs')
        plt.legend(['Basement', 'Andrew\'s room'])


# rule = matplotlib.dates.rrulewrapper(matplotlib.dates.DAILY)
# plt.xaxis.set_major_locator(loc)

# TODO: Implement nice date scales like in this example.
# 20181127/https://matplotlib.org/gallery/ticks_and_spines/date_demo_rrule.html#sphx-glr-gallery-ticks-and-spines-date-demo-rrule-py

if num_days != 0 and latest_date is not None:
    earliest_date = latest_date + relativedelta(days=-num_days)
    if data_mode == 0:
        plt.axis([matplotlib.dates.date2num(earliest_date),
                  matplotlib.dates.date2num(latest_date), 67, 77])
    else: # data_mode == 1 or data_mode == 2 or data_mode == 3
        pass

if cgi_stdout:
    if output_type == 'png':
        print 'Content-Type: image/png'
    elif output_type == 'svg':
        print 'Content-Type: image/svg+xml'
    print ''
    # plt.savefig(sys.stdout.buffer)
    plt.savefig(sys.stdout, format=output_type)
elif output_file:
    plt.savefig(output_file, format=output_type)
else:
    plt.show()
