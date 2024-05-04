import matplotlib.pyplot as plt

# 数据初始化
data = {
    '1': {'Problem Size': 1000000, 'Execution Time': 0.0534, 'Efficiency': 1.00},
    '2': {'Problem Size': 2000000, 'Execution Time': 0.0636, 'Efficiency': 1.08},
    '4': {'Problem Size': 4000000, 'Execution Time': 0.0611, 'Efficiency': 1.04},
    '8': {'Problem Size': 8000000, 'Execution Time': 0.0728, 'Efficiency': 0.95},
    '2b': {'Problem Size': 1000000, 'Execution Time': 0.0323, 'Efficiency': 0.83},
    '4b': {'Problem Size': 2000000, 'Execution Time': 0.0253, 'Efficiency': 1.36},
    '8b': {'Problem Size': 4000000, 'Execution Time': 0.0345, 'Efficiency': 0.92},
    '16b': {'Problem Size': 8000000, 'Execution Time': 0.0660, 'Efficiency': 0.52}
}

# 提取数据
x = []
y = []
sizes = []

for key, value in data.items():
    num_processes = int(key.replace('b', ''))  # 处理带 'b' 的键，表示第二组数据
    execution_time = value['Execution Time']
    problem_size = value['Problem Size']
    efficiency = value['Efficiency']
    
    x.append(num_processes)
    y.append(execution_time)
    sizes.append(problem_size)

# 绘制图表
plt.figure(figsize=(10, 6))

# 绘制 Execution Time 随 Processes 的变化曲线
plt.plot(x, y, marker='o', label='Execution Time')

# 绘制 Problem Size 随 Processes 的变化曲线
plt.plot(x, sizes, marker='s', label='Problem Size')

plt.xlabel('Number of Processes')
plt.ylabel('Value')
plt.title('Weak Scalability Experiment Results')
plt.xticks(x)
plt.legend()
plt.grid(True)

plt.show()
