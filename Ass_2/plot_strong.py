import matplotlib.pyplot as plt

# Data for Strong Scalability Experiment
processes = [1, 2, 4, 6, 8, 10, 12, 14, 16]
execution_times_strong = [0.1535, 0.0818, 0.0377, 0.0266, 0.0229, 0.0209, 0.0189, 0.0193, 0.0189]
speedup_strong = [1.0, 0.1535/0.0818, 0.1535/0.0377, 0.1535/0.0266, 0.1535/0.0229, 
                  0.1535/0.0209, 0.1535/0.0189, 0.1535/0.0193, 0.1535/0.0189]


# Plotting Strong Scalability Experiment
plt.figure(figsize=(10, 6))
plt.plot(processes, speedup_strong, marker='o', label='Strong Scalability')
plt.plot(processes, processes, '--', label='Ideal Speedup')
plt.xlabel('Number of Processes')
plt.ylabel('Speedup')
plt.title('Strong Scalability Experiment')
plt.legend()
plt.gca().set_facecolor('whitesmoke')
plt.grid(True)
plt.savefig('strong_fig.png')
plt.show()
