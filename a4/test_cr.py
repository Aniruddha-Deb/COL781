import subprocess
import matplotlib.pyplot as plt

# Run the 'timeline_test' executable and capture its output
process = subprocess.Popen(['./build/timeline_test'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
stdout, stderr = process.communicate()

# Parse the CSV output
data = []
for line in stdout.decode().splitlines():
    row = [float(x) for x in line.split(',')]
    data.append(row)

# Extract the x and y values, and the index (to use for coloring)
x_values = [row[0] for row in data]
y_values = [row[1] for row in data]
indices = list(range(len(x_values)))

# Create the scatterplot
fig, ax = plt.subplots()
scatter = ax.scatter(x_values, y_values, c=indices, cmap='viridis')

# Add labels and title
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_title('Timeline Test Results')

# Add a colorbar
cbar = fig.colorbar(scatter)
cbar.set_label('Index')

# Show the plot
plt.show()
