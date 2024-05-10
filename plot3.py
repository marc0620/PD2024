
import matplotlib.pyplot as plt
import sys

def drawRect(ax, x1, y1, x2, y2):
    rect1 = plt.Rectangle(xy=(x1, y1), width=(x2-x1), height=(y2-y1), color='b', fill=False, linewidth=1)
    ax.add_patch(rect1)

def lines2Rect(lines):
    line = lines[0].split(',')
    x1 = float(line[0])
    y1 = float(line[1])
    line = lines[2].split(',')
    x2 = float(line[0])
    y2 = float(line[1])
    return x1, y1, x2, y2

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 plot.py <filename>")
        sys.exit(0)
    fileName = sys.argv[1]
    plt.figure(figsize=(8,8))
    # get the current figure 
    fig = plt.gcf()
    # get the current Axes instance
    ax = fig.gca()
    maxX = 0
    minX = 0
    maxY = 0
    minY = 0

    with open(fileName, 'r') as f:
        lines = f.readlines()
        boundaryLines = lines[8:13]
        x1, y1, x2, y2 = lines2Rect(boundaryLines)
        rect1 = plt.Rectangle(xy=(x1, y1), width=(x2-x1), height=(y2-y1), color='r', fill=False, linewidth=1)
        ax.add_patch(rect1)
        maxX = max(maxX, x1, x2)
        minX = min(minX, x1, x2)
        maxY = max(maxY, y1, y2)
        minY = min(minY, y1, y2)

        for i in range(19, len(lines), 6):
            if i + 5 > len(lines):
                break
            boundaryLines = lines[i:i+5]
            x1, y1, x2, y2 = lines2Rect(boundaryLines)
            drawRect(ax, x1, y1, x2, y2)
            maxX = max(maxX, x1, x2)
            minX = min(minX, x1, x2)
            maxY = max(maxY, y1, y2)
            minY = min(minY, y1, y2)
       
    plt.xlim([minX, maxX])
    plt.ylim([minY, maxY])
    # save the figure 
    plt.savefig(f'plot/{fileName}.png',dpi=600)
    # show the figure
    plt.show()