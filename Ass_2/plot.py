import matplotlib.pyplot as plt

# Data for Weak Scalability Experiment
problem_sizes = ['small', 'medium', 'large']
stencil_applications = [10, 100, 10000]
execution_times = {
    'small': [0.0001, 0.0003, 0.0159],
    'medium': [0.0103, 0.0476, 0.0941],
    'large': [0.0524, 0.2643, 0.5101]
}
speedup_weak = {
    'small': [1.2, 0.7, 0.7],
    'medium': [7.0, 7.5, 7.3],
    'large': [7.0, 6.0, 6.1]
}

# Data for Strong Scalability Experiment
processes = [1, 2, 4, 6, 8, 10, 12, 14, 16]
execution_times_strong = [0.1535, 0.0818, 0.0377, 0.0266, 0.0229, 0.0209, 0.0189, 0.0193, 0.0189]
speedup_strong = [1.0, 0.1535/0.0818, 0.1535/0.0377, 0.1535/0.0266, 0.1535/0.0229, 
                  0.1535/0.0209, 0.1535/0.0189, 0.1535/0.0193, 0.1535/0.0189]

# Plotting Weak Scalability Experiment
plt.figure(figsize=(10, 6))
for i, size in enumerate(problem_sizes):
    plt.plot(stencil_applications, speedup_weak[size], label='{} problem size'.format(size))

plt.plot(stencil_applications, stencil_applications, '--', label='Ideal Speedup')
plt.xlabel('Stencil Applications')
plt.ylabel('Speedup')
plt.title('Weak Scalability Experiment')
plt.legend()
plt.grid(True)
plt.savefig('weak_fig.png')
plt.show()

# Plotting Strong Scalability Experiment
plt.figure(figsize=(10, 6))
plt.plot(processes, speedup_strong, marker='o', label='Strong Scalability')
plt.plot(processes, processes, '--', label='Ideal Speedup')
plt.xlabel('Number of Processes')
plt.ylabel('Speedup')
plt.title('Strong Scalability Experiment')
plt.legend()
plt.grid(True)
plt.savefig('strong_fig.png')
plt.show()
