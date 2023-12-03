import matplotlib.pyplot as plt
import re
import sys

def read_simulation_output(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    return lines

def parse_simulation_output(lines):
    arrivals = []
    rejections = {'09': [], '11': [], '14': [], '16': []}
    waiting = {'09': [], '11': [], '14': [], '16': []}

    for line in lines:
        match = re.match(r'(\d{3}) arrive (\d{3}) reject (\d{3}) wait-line (\d{3}) at (\d{2}):(\d{2}):(\d{2})', line)
        if match:
            minute, arrived, rejected, in_line, hour, _, _ = map(int, match.groups())
            arrivals.append((minute, arrived))

            if hour == 9:
                rejections['09'].append((minute, rejected))
                waiting['09'].append((minute, in_line))
            elif hour == 11:
                rejections['11'].append((minute, rejected))
                waiting['11'].append((minute, in_line))
            elif hour == 14:
                rejections['14'].append((minute, rejected))
                waiting['14'].append((minute, in_line))
            elif hour == 16:
                rejections['16'].append((minute, rejected))
                waiting['16'].append((minute, in_line))

    return arrivals, rejections, waiting

def plot_curve(data, title, x_label, y_label, save_path):
    plt.figure(figsize=(10, 6))
    plt.plot(*zip(*data), label=title)
    plt.title(title)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.legend()
    plt.grid(True)
    plt.savefig(save_path)
    plt.show()

def main():
    file_path = 'simulation_output.txt'
    lines = read_simulation_output(file_path)
    arrivals, rejections, waiting = parse_simulation_output(lines)

    # Plotting the curves
    plot_curve(arrivals, 'Number of Persons Arrived', 'Minute', 'Persons', 'arrivals_curve.png')
    plot_curve(rejections['09'], 'Number of Persons Rejected (9:00 AM - 10:59 AM)', 'Minute', 'Persons', 'rejections_09_curve.png')
    plot_curve(rejections['11'], 'Number of Persons Rejected (11:00 AM - 1:59 PM)', 'Minute', 'Persons', 'rejections_11_curve.png')
    plot_curve(rejections['14'], 'Number of Persons Rejected (2:00 PM - 3:59 PM)', 'Minute', 'Persons', 'rejections_14_curve.png')
    plot_curve(rejections['16'], 'Number of Persons Rejected (4:00 PM - 6:59 PM)', 'Minute', 'Persons', 'rejections_16_curve.png')
    plot_curve(waiting['09'], 'Number of Persons Waiting (9:00 AM - 10:59 AM)', 'Minute', 'Persons', 'waiting_09_curve.png')
    plot_curve(waiting['11'], 'Number of Persons Waiting (11:00 AM - 1:59 PM)', 'Minute', 'Persons', 'waiting_11_curve.png')
    plot_curve(waiting['14'], 'Number of Persons Waiting (2:00 PM - 3:59 PM)', 'Minute', 'Persons', 'waiting_14_curve.png')
    plot_curve(waiting['16'], 'Number of Persons Waiting (4:00 PM - 6:59 PM)', 'Minute', 'Persons', 'waiting_16_curve.png')

if __name__ == "__main__":
    main()
