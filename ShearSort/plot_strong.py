import matplotlib.pyplot as plt

# Data
process_num = list(range(1, 17))
speedup = [1.00, 1.97, 2.95, 3.96, 4.88, 5.77, 6.69, 7.65, 8.40, 9.37, 10.26, 11.44, 12.29, 13.29, 14.38, 15.27]
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
