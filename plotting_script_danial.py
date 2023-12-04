import os
import re
import matplotlib.pyplot as plt

def read_simulation_output(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    return lines

def parse_simulation_output(lines):
    arrivals = []
    rejectionArray = []
    waitingArray = []
    rejections = {'09': [], '11': [], '14': [], '16': []}
    waiting = {'09': [], '11': [], '14': [], '16': []}

    for line in lines:
        match = re.match(r'(\d{3}) arrive (\d{3}) reject (\d{3}) wait-line (\d{3}) at (\d{2}):(\d{2}):(\d{2})', line)
        if match:
            minute, arrived, rejected, in_line, hour, _, _ = map(int, match.groups())
            arrivals.append((minute, arrived))
            rejectionArray.append((minute, rejected))
            waitingArray.append((minute, in_line))

            if 0 <= hour <= 1:
                rejections['09'].append((minute, rejected))
                waiting['09'].append((minute, in_line))
            elif 2 <= hour <= 4:
                rejections['11'].append((minute, rejected))
                waiting['11'].append((minute, in_line))
            elif 5 <= hour <= 6:
                rejections['14'].append((minute, rejected))
                waiting['14'].append((minute, in_line))
            elif hour >= 7:
                rejections['16'].append((minute, rejected))
                waiting['16'].append((minute, in_line))

    return arrivals, rejections, waiting, rejectionArray, waitingArray

def accumulate_data(directory_path):
    all_data = {}

    files = [file for file in os.listdir(directory_path) if file.startswith('simulation_output_') and file.endswith('.txt')]

    for file in files:
        file_path = os.path.join(directory_path, file)
        lines = read_simulation_output(file_path)
        arrivals, rejections, waiting, rejectionArray, waitingArray = parse_simulation_output(lines)

        file_data = {'Arrivals': arrivals, 'Rejections': rejectionArray, 'Waiting': waitingArray}
        all_data[file] = file_data

    return all_data


def plot_combined_curves(data, title, x_label, y_label, save_path):
    plt.figure(figsize=(10, 6))

    for file_name, file_data in data.items():
        for key, values in file_data.items():
            plt.plot(*zip(*values), label=f'{file_name}')

    plt.title(title)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.legend()
    plt.grid(True)
    plt.savefig(save_path)
    plt.show()


def main():
    directory_path = os.getcwd()
    all_data = accumulate_data(directory_path)

    # Plotting all arrivals for each file on the same graph
    arrivals_data = {file_name: {'Arrivals': file_data['Arrivals']} for file_name, file_data in all_data.items()}
    plot_combined_curves(arrivals_data, 'Number of Persons Arrived', 'Minute', 'Persons', 'combined_arrivals_curve.png')

    # Plotting all rejections for each file on the same graph
    rejections_data = {file_name: {'Rejected': file_data['Rejections']} for file_name, file_data in all_data.items()}
    plot_combined_curves(rejections_data, 'Number of Persons Rejected', 'Minute', 'Persons', 'combined_rejections_curve.png')

    # Plotting all waiting for each file on the same graph
    waiting_data = {file_name: {'Waiting': file_data['Rejections']} for file_name, file_data in all_data.items()}
    plot_combined_curves(waiting_data, 'Number of Persons Waiting', 'Minute', 'Persons', 'combined_waiting_curve.png')

if __name__ == "__main__":
    main()

