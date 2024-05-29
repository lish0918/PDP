import matplotlib.pyplot as plt

# 进程数
process_numbers = [1, 4, 9, 16, 25, 36, 49, 64]

# Efficiency 数据
efficiency = [1.00, 1.05, 1.02, 0.86, 0.68, 0.55, 0.43, 0.33]

# 理想 Efficiency，均为 1
ideal_efficiency = [1] * len(process_numbers)

# 画图
plt.plot(process_numbers, efficiency, marker='o', label='Actual Efficiency')
plt.plot(process_numbers, ideal_efficiency, 'r--', label='Ideal Efficiency')

# 添加标签和标题
plt.xlabel('Process Number')
plt.ylabel('Efficiency')
plt.title('Trend of Weak Scaling of Shear Sort Algorithm')

# 添加图例
plt.legend()

# 显示图形
plt.grid(True)
plt.gca().set_facecolor('whitesmoke')
plt.xticks(process_numbers)
plt.yticks([i/10 for i in range(11)])  # 设置纵轴刻度为0到1，每0.1一个刻度
plt.show()
