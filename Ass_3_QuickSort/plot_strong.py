import matplotlib.pyplot as plt

# 定义数据
data = {
    'Strategy': [1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3],
    'Process_Number': [1, 2, 4, 8, 16] * 3,
    'Speedup': [1.00, 1.92, 2.95, 4.36, 7.44,
                1.00, 1.92, 2.97, 4.35, 7.44,
                1.00, 1.91, 2.94, 4.33, 7.25]
}

# 计算理想加速比（基于进程数）
ideal_speedup = data['Process_Number']

# 绘制加速比和理想加速比的对比图
fig, ax = plt.subplots(figsize=(10, 6))

# 根据策略分组绘制实际加速比曲线
for name, start_idx in zip(['Strategy 1', 'Strategy 2', 'Strategy 3'], [0, 5, 10]):
    group = data['Speedup'][start_idx:start_idx+5]  # 提取当前策略的实际加速比数据
    process_numbers = data['Process_Number'][start_idx:start_idx+5]  # 提取对应的进程数
    ax.plot(process_numbers, group, marker='o', label=name)

# 根据进程数绘制理想加速比曲线
ax.plot(data['Process_Number'][:5], ideal_speedup[:5], linestyle='--', label='Ideal Speedup')

# 设置图表标题和标签
ax.set_title('Speedup vs. Process Number for Different Strategies')
ax.set_xlabel('Number of Threads (Process Number)')
ax.set_ylabel('Speedup')
ax.legend()

# 显示图表
plt.grid(True)
plt.gca().set_facecolor('whitesmoke')
plt.xticks(data['Process_Number'][:5])  # 设置x轴刻度为1到16
plt.show()
