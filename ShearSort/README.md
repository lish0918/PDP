# Shear Sort 并行实现

### 申请资源

在 Snowy 上申请资源：

```bash
interactive -M snowy -A uppmax2024-2-9 -n 16 -t 04:10:00
```

### 加载软件环境

```bash
module load gcc/12.2.0 openmpi/4.1.4
```

### 编译代码

```bash
make shearshort
```

### 执行程序

```bash
mpirun -n 16 ./shear_sort_mpi <matrix_size> <input_file> <output_file>
```

其中，`<matrix_size>` 是矩阵的大小，`<input_file>` 是输入数据文件的路径，`<output_file>` 是排序后的输出文件路径。