# visualize.py
import matplotlib.pyplot as plt

def visualize_data(filename):
    x = []
    y = []
    with open(filename, 'r') as f:
        for line in f:
            index, value = line.strip().split()
            x.append(int(index))
            y.append(float(value))
    
    plt.figure()
    plt.plot(x, y)
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title('Visualization of Data')
    plt.savefig('visualization.png')
    plt.show()

if __name__ == '__main__':
    filename = 'data.txt'
    visualize_data(filename)
