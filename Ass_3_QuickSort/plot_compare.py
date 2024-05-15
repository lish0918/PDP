import matplotlib.pyplot as plt

# 定义数据
data = {
    'Strategy': ['Strategy 1', 'Strategy 1', 'Strategy 1', 'Strategy 1', 'Strategy 2', 'Strategy 2', 'Strategy 2', 'Strategy 2', 'Strategy 3', 'Strategy 3', 'Strategy 3', 'Strategy 3'],
    'Order': ['Original', 'Descending', 'Original', 'Descending', 'Original', 'Descending', 'Original', 'Descending', 'Original', 'Descending', 'Original', 'Descending'],
    'Process_Number': [1, 4, 8, 1, 4, 8, 1, 4, 8, 1, 4, 8, 1, 4, 8, 1, 4, 8],
    'Speedup': [1.00, 2.95, 4.36, 1.00, 2.21, 2.90, 1.00, 2.97, 4.35, 1.00, 1.91, 2.48, 1.00, 2.94, 4.33, 1.00, 1.89, 2.48]
}

# 提取数据绘制图表
fig, ax = plt.subplots(figsize=(10, 6))

# 策略1 - 原顺序
strategy1_original = data['Speedup'][:3]
process_numbers1_original = data['Process_Number'][:3]
ax.plot(process_numbers1_original, strategy1_original, marker='o', linestyle='-', label='Strategy 1 - Original Order')

# 策略1 - 逆序
strategy1_descending = data['Speedup'][3:6]
process_numbers1_descending = data['Process_Number'][3:6]
ax.plot(process_numbers1_descending, strategy1_descending, marker='o', linestyle='--', label='Strategy 1 - Descending Order')

# 策略2 - 原顺序
strategy2_original = data['Speedup'][6:9]
process_numbers2_original = data['Process_Number'][6:9]
ax.plot(process_numbers2_original, strategy2_original, marker='o', linestyle='-', label='Strategy 2 - Original Order')

# 策略2 - 逆序
strategy2_descending = data['Speedup'][9:12]
process_numbers2_descending = data['Process_Number'][9:12]
ax.plot(process_numbers2_descending, strategy2_descending, marker='o', linestyle='--', label='Strategy 2 - Descending Order')

# 策略3 - 原顺序
strategy3_original = data['Speedup'][12:15]
process_numbers3_original = data['Process_Number'][12:15]
ax.plot(process_numbers3_original, strategy3_original, marker='o', linestyle='-', label='Strategy 3 - Original Order')

# 策略3 - 逆序
strategy3_descending = data['Speedup'][15:18]
process_numbers3_descending = data['Process_Number'][15:18]
ax.plot(process_numbers3_descending, strategy3_descending, marker='o', linestyle='--', label='Strategy 3 - Descending Order')

# 设置图表标题和标签
ax.set_title('Descending Order vs Original Order')
ax.set_xlabel('Process Number')
ax.set_ylabel('Speedup')

# 添加图例
ax.legend()

# 显示图表
plt.grid(True)
plt.gca().set_facecolor('whitesmoke')
plt.show()
