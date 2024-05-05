import matplotlib.pyplot as plt

# 提取数据并计算效率
problem_sizes = [1000000, 2000000, 4000000, 8000000]
efficiency_1mil = [1.00,1.08,1.04,0.95]
efficiency_5mil = [0.83,1.36,0.92,0.52]

# 绘制效率图
plt.figure(figsize=(10, 6))
plt.plot(problem_sizes, efficiency_1mil, marker='o', label='10,000,000 per Process')
plt.plot(problem_sizes, efficiency_5mil, marker='s', label='5,000,000 per Process')
plt.plot(problem_sizes, [1]*4, '--', label='Ideal Efficiency')
plt.xlabel('Problem Size')
plt.ylabel('Efficiency')
plt.ylim(0, 1.5)
plt.title('Efficiency with Problem Size and Processes Increase')
plt.xticks(problem_sizes)
plt.legend()
plt.gca().set_facecolor('whitesmoke')
plt.grid(True)
plt.show()
