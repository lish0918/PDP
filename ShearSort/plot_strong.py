import matplotlib.pyplot as plt

# Data
process_num = list(range(1, 17))
speedup = [1, 1.731765152, 2.460821004, 3.177129752, 3.724208477, 4.185460974, 4.591738389, 4.96755463, 5.396365545, 5.991427059, 6.211234494, 6.429456143, 6.62705926, 6.792513102, 6.916622266, 7.066189327]
ideal_speedup = process_num  # Ideal speedup

# Plotting the speedup
plt.figure(figsize=(10, 6))
plt.plot(process_num, speedup, marker='o', label='Actual Speedup')
plt.plot(process_num, ideal_speedup, linestyle='--', label='Ideal Speedup')

# Adding titles and labels
plt.title('Trend of Strong Scaling of Shear Sort Algorithm')
plt.xlabel('Process Number')
plt.ylabel('Speedup')
plt.legend()

# Display the plot
plt.grid(True)
plt.gca().set_facecolor('whitesmoke')
plt.show()
