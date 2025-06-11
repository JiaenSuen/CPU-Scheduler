import matplotlib.pyplot as plt
import math

def visualize_data(filename):
    x = []
    y1 = []
    y2 = []

    with open(filename, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 3:
                index = int(parts[0])
                val1 = float(parts[1]) if parts[1] != "nan" else math.nan
                val2 = float(parts[2]) if parts[2] != "nan" else math.nan

                x.append(index)
                y1.append(val1)
                y2.append(val2)

    plt.figure()
    plt.plot(x, y1, label='Global  Best', marker='o')
    plt.plot(x, y2, label='Pop Avg Best', marker='x')
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title('Visualization of One or Two Data Sets')
    plt.legend()
    plt.savefig('visualization.png')
    plt.show()

if __name__ == '__main__':
    filename = 'data.txt'
    visualize_data(filename)
