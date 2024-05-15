import matplotlib.pyplot as plt

# 定义数据
data = {
    'Pivot Strategy': [1, 1, 1, 2, 2, 2, 3, 3, 3],
    'Process_Number': [2, 4, 8, 2, 4, 8, 2, 4, 8],
    'Input_Size': [1, 2, 3] * 3,  # 使用简化的输入大小范围 1 到 3
    'Efficiency': [0.96, 0.74, 0.53,
                   0.96, 0.74, 0.53,
                   0.96, 0.73, 0.53]
}

# 绘制弱可扩展性结果图
fig, ax = plt.subplots(figsize=(10, 6))

# 根据策略分组绘制实际效率曲线
for strategy in [1, 2, 3]:
    strategy_efficiency = data['Efficiency'][3 * (strategy - 1):3 * strategy]  # 提取当前策略的效率数据
    process_numbers = data['Process_Number'][3 * (strategy - 1):3 * strategy]  # 提取对应的进程数
    label = f'Strategy {strategy}'
    ax.plot(process_numbers, strategy_efficiency, marker='o', label=label)

# 设置图表标题和标签
ax.set_title('Weak Scalability')
ax.set_xlabel('Process Number')
ax.set_ylabel('Efficiency')

# 设置纵坐标范围为0到1
ax.set_ylim(0, 1)

# 显示图例
ax.legend()

# 显示图表
plt.grid(True)
plt.gca().set_facecolor('whitesmoke')
plt.show()
